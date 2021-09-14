#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <atomic>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>
#include "SharedQueue.h"

// Original inspiration from https://github.com/progschj/ThreadPool/blob/master/ThreadPool.h

/**
 * @brief The ThreadPool class allows to push task into a pool of thread
 * The task to push must have the type std::function<void(int id)>
 * The int id passed to the function will be the id of thread (useful for
 * resource management for non reentrant stuff)
 *
 * USAGE :
 * ThreadPool p(4);
 * std::vector<std::future<void(int id)>> results(4);
 *  for (int i = 0; i < 8; ++i) { // for 8 iterations,
 *      for (int j = 0; j < 4; ++j) {
 *          results[j] = p.push([&arr, j](int){ arr[j] +=2; }); //function must take int - id of thread as param
 *      }
 *      for (int j = 0; j < 4; ++j) {
 *          results[j].get();
 *      }
 *      arr[4] = std::min_element(arr, arr + 4);
    }
 */


class ThreadPool {

public:

    ThreadPool() { init(); }

    ThreadPool(int nThreads) {
        init();
        resize(nThreads);
    }

    // the destructor waits for all the functions in the queue to be finished
    ~ThreadPool() {
        stop(true);
    }

    // get the number of running threads in the pool
    int size() { return static_cast<int>(m_threads.size()); }

    // get the number of task in queue
    int queue_size() { return m_queue.size(); }

    // number of idle threads
    int n_idle() { return m_nWaiting; }
    std::thread & get_thread(int i) { return *m_threads[i]; }

    // change the number of threads in the pool
    // should be called from one thread, otherwise be careful to not interleave, also with stop()
    // nThreads must be >= 0
    void resize(int nThreads) {
        if (!m_isStop && !m_isDone) {
            int oldNThreads = static_cast<int>(m_threads.size());
            if (oldNThreads <= nThreads) {  // if the number of threads is increased
                m_threads.resize(nThreads);
                m_flags.resize(nThreads);

                for (int i = oldNThreads; i < nThreads; ++i) {
                    m_flags[i] = std::make_shared<std::atomic<bool>>(false);
                    set_thread(i);
                }
            } else {  // the number of threads is decreased
                for (int i = oldNThreads - 1; i >= nThreads; --i) {
                    *m_flags[i] = true;  // this thread will finish
                    m_threads[i]->detach();
                }
                {
                    // stop the detached threads that were waiting
                    std::unique_lock<std::mutex> lock(m_mutex);
                    m_cv.notify_all();
                }
                m_threads.resize(nThreads);  // safe to delete because the threads are detached
                m_flags.resize(nThreads);  // safe to delete because the threads have copies of shared_ptr of the flags, not originals
            }
        }
    }

    // empty the queue
    void clear_queue() {
        std::function<void(int id)> * _f;
        while (m_queue.pop_front_no_wait(_f))
            delete _f; // empty the queue
    }

    // pops a functional wrapper to the original function
    std::function<void(int)> pop() {
        std::function<void(int id)> * _f = nullptr;
        m_queue.pop_front_no_wait(_f);
        std::unique_ptr<std::function<void(int id)>> func(_f); // at return, delete the function even if an exception occurred
        std::function<void(int)> f;
        if (_f)
            f = *_f;
        return f;
    }

    // wait for all computing threads to finish and stop all threads
    // may be called asynchronously to not pause the calling thread while waiting
    // if isWait == true, all the functions in the queue are run, otherwise the queue is cleared without running the functions
    void stop(bool isWait = false) {
        if (!isWait) {
            if (m_isStop)
                return;
            m_isStop = true;
            for (int i = 0, n = size(); i < n; ++i) {
                *m_flags[i] = true;  // command the threads to stop
            }
            clear_queue();  // empty the queue
        }
        else {
            if (m_isDone || m_isStop)
                return;
            m_isDone = true;  // give the waiting threads a command to finish
        }
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_cv.notify_all();  // stop all waiting threads
        }
        for (int i = 0; i < static_cast<int>(m_threads.size()); ++i) {  // wait for the computing threads to finish
            if (m_threads[i]->joinable())
                m_threads[i]->join();
        }
        // if there were no threads in the pool but some functors in the queue, the functors are not deleted by the threads
        // therefore delete them here
        clear_queue();
        m_threads.clear();
        m_flags.clear();
    }

    template<typename F, typename... Rest>
    auto push(F && f, Rest&&... rest) ->std::future<decltype(f(0, rest...))> {
        auto pck = std::make_shared<std::packaged_task<decltype(f(0, rest...))(int)>>(std::bind(std::forward<F>(f),
                                                                                                std::placeholders::_1,
                                                                                                std::forward<Rest>(rest)...)
                                                                                      );
        auto _f = new std::function<void(int id)>([pck](int id) {
            (*pck)(id);
        });
        m_queue.push_back(_f);
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.notify_one();
        return pck->get_future();
    }

    // run the user's function that accepts argument int - id of the running thread. returned value is templatized
    // operator returns std::future, where the user can get the result and rethrow the catched exceptions
    template<typename F>
    auto push(F && f) ->std::future<decltype(f(0))> {
        auto pck = std::make_shared<std::packaged_task<decltype(f(0))(int)>>(std::forward<F>(f));
        auto _f = new std::function<void(int id)>([pck](int id) {
            (*pck)(id);
        });
        m_queue.push_back(_f);
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cv.notify_one();
        return pck->get_future();
    }


private:

    // deleted
    ThreadPool(const ThreadPool &);// = delete;
    ThreadPool(ThreadPool &&);// = delete;
    ThreadPool & operator=(const ThreadPool &);// = delete;
    ThreadPool & operator=(ThreadPool &&);// = delete;

    void set_thread(int i) {
        std::shared_ptr<std::atomic<bool>> flag(m_flags[i]); // a copy of the shared ptr to the flag
        auto f = [this, i, flag/* a copy of the shared ptr to the flag */]() {
            std::atomic<bool> & _flag = *flag;
            std::function<void(int id)> * _f;
            bool isPop = m_queue.pop_front_no_wait(_f);
            while (true) {
                while (isPop) {  // if there is anything in the queue
                    std::unique_ptr<std::function<void(int id)>> func(_f); // at return, delete the function even if an exception occurred
                    (*_f)(i);
                    if (_flag)
                        return;  // the thread is to be stopped, return even if the queue is not empty yet
                    else
                        isPop = m_queue.pop_front_no_wait(_f);
                }
                // the queue is empty here, wait for the next command
                std::unique_lock<std::mutex> lock(m_mutex);
                ++m_nWaiting;
                m_cv.wait(lock, [this, &_f, &isPop, &_flag](){ isPop = m_queue.pop_front_no_wait(_f); return isPop || m_isDone || _flag; });
                --m_nWaiting;
                if (!isPop)
                    return;  // if the queue is empty and isDone == true or *flag then return
            }
        };
        m_threads[i].reset(new std::thread(f));
    }

    void init() { m_nWaiting = 0; m_isStop = false; m_isDone = false; }

    std::vector<std::unique_ptr<std::thread>> m_threads;
    std::vector<std::shared_ptr<std::atomic<bool>>> m_flags;
    SharedQueue<std::function<void(int id)> *> m_queue;
    std::atomic<bool> m_isDone;
    std::atomic<bool> m_isStop;
    std::atomic<int> m_nWaiting;  // how many threads are waiting

    std::mutex m_mutex;
    std::condition_variable m_cv;
};

#endif //THREADPOOL_H

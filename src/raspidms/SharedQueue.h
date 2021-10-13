#ifndef SHAREDQUEUE_H
#define SHAREDQUEUE_H

#include <chrono>
#include <condition_variable>
#include <queue>
#include <mutex>

template <typename T>
class SharedQueue
{
public:
    SharedQueue();
    ~SharedQueue();

    //return ref to front element
    //MAY BLOCK until there is an element to retrieve
    T& front_wait();

    //return ref to front element
    //not blocking but may not fill the element at all
    //(return false in that case)
    bool front_no_wait(T& item);

    //remove front element (not blocking)
    //return true if an element was deleted
    //false if already empty
    bool pop_front_no_wait();

    //retrieve and remove front element (not blocking)
    //return true if an element was retrieve and item modified
    //false overwise (queue empty and item not modified)
    bool pop_front_no_wait(T& item);

    //retrieve and remove front element :
    //MAY_BLOCK until an element is retrieved
    void pop_front_wait(T& item);

    //push an element at back
    void push_back(const T& item);
    void push_back(T&& item);

    int size();
    bool empty();

private:
    std::deque<T> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_cv;
};

template <typename T>
SharedQueue<T>::SharedQueue(){}

template <typename T>
SharedQueue<T>::~SharedQueue(){}

template <typename T>
T& SharedQueue<T>::front_wait()
{
    std::unique_lock<std::mutex> mlock(m_mutex);
    while (m_queue.empty())
    {
        m_cv.wait(mlock);
    }
    return m_queue.front();
}

template <typename T>
bool SharedQueue<T>::front_no_wait(T& item)
{
    std::unique_lock<std::mutex> mlock(m_mutex);
    if (m_queue.empty())
        return false;
    item = m_queue.front();
    return true;
}

template <typename T>
bool SharedQueue<T>::pop_front_no_wait()
{
    std::unique_lock<std::mutex> mlock(m_mutex);
    if (m_queue.empty())
        return false;
    m_queue.pop_front();
    return true;
}

template <typename T>
bool SharedQueue<T>::pop_front_no_wait(T& item)
{
    std::unique_lock<std::mutex> mlock(m_mutex);
    if (m_queue.empty())
        return false;
    item = m_queue.front();
    m_queue.pop_front();
    return true;
}

template <typename T>
void SharedQueue<T>::pop_front_wait(T& item)
{
    std::unique_lock<std::mutex> mlock(m_mutex);
    while (m_queue.empty())
    {
        m_cv.wait(mlock);
    }
    item = m_queue.front();
    m_queue.pop_front();
}

template <typename T>
void SharedQueue<T>::push_back(const T& item)
{
    std::unique_lock<std::mutex> mlock(m_mutex);
    m_queue.push_back(item);
    mlock.unlock();     // unlock before notification to minimize mutex con
    m_cv.notify_one(); // notify one waiting thread
}

template <typename T>
void SharedQueue<T>::push_back(T&& item)
{
    std::unique_lock<std::mutex> mlock(m_mutex);
    m_queue.push_back(std::move(item));
    mlock.unlock();     // unlock before notification to minimize mutex con
    m_cv.notify_one(); // notify one waiting thread
}

template <typename T>
int SharedQueue<T>::size()
{
    std::unique_lock<std::mutex> mlock(m_mutex);
    int size = m_queue.size();
    mlock.unlock();
    return size;
}

template <typename T>
bool SharedQueue<T>::empty()
{
    std::unique_lock<std::mutex> mlock(m_mutex);
    return m_queue.empty();
}

#endif //SHAREDQUEUE_H

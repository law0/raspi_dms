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
    std::deque<T> queue_;
    std::mutex mutex_;
    std::condition_variable cond_;
};

template <typename T>
SharedQueue<T>::SharedQueue(){}

template <typename T>
SharedQueue<T>::~SharedQueue(){}

template <typename T>
T& SharedQueue<T>::front_wait()
{
    std::unique_lock<std::mutex> mlock(mutex_);
    while (queue_.empty())
    {
        cond_.wait(mlock);
    }
    return queue_.front();
}

template <typename T>
bool SharedQueue<T>::pop_front_no_wait()
{
    std::unique_lock<std::mutex> mlock(mutex_);
    if (queue_.empty())
        return false;
    queue_.pop_front();
    return true;
}

template <typename T>
bool SharedQueue<T>::pop_front_no_wait(T& item)
{
    std::unique_lock<std::mutex> mlock(mutex_);
    if (queue_.empty())
        return false;
    item = queue_.front();
    queue_.pop_front();
    return true;
}

template <typename T>
void SharedQueue<T>::pop_front_wait(T& item)
{
    std::unique_lock<std::mutex> mlock(mutex_);
    while (queue_.empty())
    {
        cond_.wait(mlock);
    }
    item = queue_.front();
    queue_.pop_front();
}

template <typename T>
void SharedQueue<T>::push_back(const T& item)
{
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push_back(item);
    mlock.unlock();     // unlock before notification to minimize mutex con
    cond_.notify_one(); // notify one waiting thread
}

template <typename T>
void SharedQueue<T>::push_back(T&& item)
{
    std::unique_lock<std::mutex> mlock(mutex_);
    queue_.push_back(std::move(item));
    mlock.unlock();     // unlock before notification to minimize mutex con
    cond_.notify_one(); // notify one waiting thread
}

template <typename T>
int SharedQueue<T>::size()
{
    std::unique_lock<std::mutex> mlock(mutex_);
    int size = queue_.size();
    mlock.unlock();
    return size;
}

template <typename T>
bool SharedQueue<T>::empty()
{
    std::unique_lock<std::mutex> mlock(mutex_);
    return queue_.empty();
}

#endif //SHAREDQUEUE_H

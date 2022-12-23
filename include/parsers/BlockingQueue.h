/**
 * BlockingQueue.h
 * 
 * Thread safe blocking queue
 */

#ifndef __BLOCKING_QUEUE_H__
#define __BLOCKING_QUEUE_H__

#include <functional>
#include <mutex>
#include <queue>

template <typename T>
class BlockingQueue
{
private:
    std::mutex mutex_;
    std::queue<T> queue_;
public:
    T pop()
    {
        const std::lock_guard<std::mutex> lock(mutex_);
        T value = queue_.front();
        queue_.pop();
        return value;
    }

    void push(T value)
    {
        const std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(value);
    }

    T front()
    {
        const std::lock_guard<std::mutex> lock(mutex_);
        return queue_.front();
    }

    size_t size()
    {
        const std::lock_guard<std::mutex> lock(mutex_);
        size_t size = queue_.size();
        return size;
    }

    bool empty() {
        const std::lock_guard<std::mutex> lock(mutex_);
        bool check = queue_.empty();
        return check;
    }
};

#endif

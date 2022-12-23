/**
 * BlockingVector.h
 * 
 * Thread safe blocking Vector
 */

#ifndef __BLOCKING_QUEUE_H__
#define __BLOCKING_QUEUE_H__

#include <functional>
#include <mutex>
#include <vector>

template <typename T>
class BlockingVector
{
private:
    std::mutex mutex_;
    std::vector<T> vector_;
public:

    decltype(auto) operator[](size_t index)
    {
        const std::lock_guard<std::mutex> lock(mutex_);
        return vector_[index];
    }

    decltype(auto) emplace_back(T element)
    {
        const std::lock_guard<std::mutex> lock(mutex_);
        return vector_.emplace_back(T);
    }

    decltype(auto) erase(size_t index)
    {
        const std::lock_guard<std::mutex> lock(mutex_);
        return vector_.erase(index);
    }

    size_t size()
    {
        const std::lock_guard<std::mutex> lock(mutex_);
        return vector_.size();
    }

    bool empty() {
        const std::lock_guard<std::mutex> lock(mutex_);
        return vector_.empty();;
    }
};

#endif

#pragma once
#include <mutex>

template<typename T>
class LockQueue {
public:
    LockQueue() { }
    LockQueue(const LockQueue&) = delete;
    LockQueue& operator=(const LockQueue&) = delete;

    void Push(T value)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push(std::move(value));
        _condVar.notify_one();
    }

    bool TryPop(T& value)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_queue.empty())
            return false;

        value = std::move(_queue.front());
        _queue.pop();
        return true;
    }

    void WaitPop(T& value)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _condVar.wait(lock, [this] { return !_queue.empty(); });
        value = std::move(_queue.top());
        _queue.pop();
    }

private:
    std::queue<T> _queue;
    std::mutex _mutex;
    std::condition_variable _condVar; 
};

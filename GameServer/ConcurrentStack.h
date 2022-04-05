#pragma once

#include <mutex>

template<typename T>
class LockStack {
public:
    LockStack() { }

    LockStack(const LockStack&) = delete;
    LockStack& operator=(const LockStack&) = delete;

    void Push(T value)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _stack.push(std::move(value));
        _condVar.notify_one();
    }

    bool TryPop(T& value)
    {
        std::lock_guard<std::mutex> lock(_mutex);
        if (_stack.empty())
            return false;

        value = std::move(_stack.top());
        _stack.pop();
        return true;
    }

    void WaitPop(T& value)
    {
        std::unique_lock<std::mutex> lock(_mutex);
        _condVar.wait(lock, [this] { return !_stack.empty(); });
        value = std::move(_stack.top());
        _stack.pop();
    }

private:
    std::stack<T> _stack;
    std::mutex _mutex;
    std::condition_variable _condVar;
};

#pragma once

#include <mutex>
#include <atomic>

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

template <typename T>
class LockFreeStack {
    struct Node;

    struct CountedNodePtr {
        int32 externalCount = 0;
        Node* ptr = nullptr;
    };
    
    struct Node {
        Node(const T& value)
            : data(std::make_shared<T>(value))
        {
        }
        std::shared_ptr<T> data;
        std::atomic<int32> internalCount = 0;
        CountedNodePtr next;
    };

public:
    void Push(const T& value)
    {
        CountedNodePtr node;
        node.ptr = new Node(value);
        node.externalCount = 1;

        node.ptr->next = _head;
        while (_head.compare_exchange_weak(node.ptr->next, node) == false) {
            ;
        }

    }

    std::shared_ptr<T> TryPop()
    {
        CountedNodePtr oldHead = _head;
        while (true) {
            IncreaseHeadCount(oldHead);
            // externalCount >= 2 -> safe to reference
            Node *ptr = oldHead.ptr;

            if (ptr == nullptr)
                return std::shared_ptr<T>();

            if (_head.compare_exchange_strong(oldHead, ptr->next)) {
                std::shared_ptr<T> res;
                res.swap(ptr->data);

                const int32 countIncrease = oldHead.externalCount - 2;
                if (ptr->internalCount.fetch_add(countIncrease) == -countIncrease)
                    delete ptr;

                return res;
            }
            // Remaining thread which lose pop-race
            else if (ptr->internalCount.fetch_sub(1) == 1){
                // Last thread which lose pop-race is responsible to delete ptr
                delete ptr;
            }
        }
    }

private:
    void IncreaseHeadCount(CountedNodePtr& oldCounter)
    {
        while (true) {
            CountedNodePtr newCounter = oldCounter;
            newCounter.externalCount++;

            if (_head.compare_exchange_strong(oldCounter, newCounter)) {
                oldCounter.externalCount = newCounter.externalCount;
                break;
            }
        }
    }

private:
    std::atomic<CountedNodePtr> _head;
};

/*
// share_ptr is not lock-free -> This class is not genuinely lock-free
template<typename T>
class LockFreeStack {
    struct Node {
        Node(const T& value)
            : data(std::make_shared<T>(value)), next(nullptr)
        {
        }
        std::shared_ptr<T> data;
        std::shared_ptr<Node> next;
    };

public:
    void Push(const T& value)
    {
        std::shared_ptr<Node> node = std::make_shared<Node>(value);
        node->next = std::atomic_load(&_head);

        while (std::atomic_compare_exchange_weak(&_head, &node->next, node) == false) {
            ;
        }
    }

    std::shared_ptr<T> TryPop()
    {
        std::shared_ptr<Node> oldHead = std::atomic_load(&_head);

        while (oldHead && std::atomic_compare_exchange_weak(&_head, &oldHead, oldHead->next) == false) {
            ;
        }

        if (oldHead == nullptr)
            return std::shared_ptr<T>();
        
        return oldHead->data;
    }

private:
    std::shared_ptr<Node> _head;
};
*/
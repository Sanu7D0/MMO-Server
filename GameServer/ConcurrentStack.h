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

template<typename T>
class LockFreeStack {
    struct Node {
        Node(const T& value)
            : data(value), next(nullptr)
        {
        }
        T data;
        Node* next;
    };

public:
    void Push(const T& value)
    {
        Node* node = new Node(value);
        node->next = _head;

        while (_head.compare_exchange_weak(node->next, node) == false) {
            ;
        }
    }

    bool TryPop(T& value)
    {
        ++_popCount;

        Node* oldHead = _head;
        while (oldHead && _head.compare_exchange_weak(oldHead, oldHead->next) == false) {
            ;
        }

        if (oldHead == nullptr) {
            --_popCount;
            return false;
        }

        value = oldHead->data;
        TryDelete(oldHead);

        // delete oldHead;

        return true;
    }

    void TryDelete(Node* oldHead)
    {
        if (_popCount == 1) {
            Node* node = _pendingList.exchange(nullptr);

            if (--_popCount == 0) {
                DeleteNodes(node);
            } else if (node) {
                ChainPendingNodeList(node);
            }

            delete oldHead;
        }
        else {
            ChainPendingNode(oldHead);
            --_popCount;
        }
    }

    void ChainPendingNodeList(Node* first, Node* last)
    {
        last->next = _pendingList;

        while (_pendingList.compare_exchange_weak(last->next, first) == false) {
            ;
        }
    }

    void ChainPendingNodeList(Node* node)
    {
        Node* last = node;
        while (last->next)
            last = last->next;

        ChainPendingNodeList(node, last);
    }

    void ChainPendingNode(Node* node)
    {
        ChainPendingNodeList(node, node);
    }

    static void DeleteNodes(Node* node)
    {
        while (node) {
            Node* next = node->next;
            delete node;
            node = next;
        }
    }

private:
    std::atomic<Node*> _head;

    // Pop Count Strategy
    std::atomic<int32> _popCount = 0;
    std::atomic<Node*> _pendingList;
};
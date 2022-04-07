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

template<typename T>
class LockFreeQueue {
    struct Node;

    struct CountedNodePtr {
        int32 externalCount;
        Node* ptr = nullptr;
    };

    struct NodeCounter {
        uint32 internalCount : 30; // 30 bit
        uint32 externalCountRemaining : 2; // 2 bit
    };

    struct Node {
        std::atomic<T*> data;
        std::atomic<NodeCounter> count;
        CountedNodePtr next;

        Node()
        {
            NodeCounter newCount;
            newCount.internalCount = 0;
            newCount.externalCountRemaining = 2;
            count.store(newCount);

            next.ptr = nullptr;
            next.externalCount = 0;
        }

        void ReleaseRef()
        {
            NodeCounter oldCounter = count.load();

            while (true) {
                NodeCounter newCounter = oldCounter;
                newCounter.internalCount--;

                if (count.compare_exchange_strong(oldCounter, newCounter)) {
                    if (newCounter.internalCount == 0 && newCounter.externalCountRemaining == 0)
                        delete this;

                    break;
                }
            }
        }
    };

public:
    LockFreeQueue()
    {
        CountedNodePtr node;
        node.ptr = new Node;
        node.externalCount = 1;

        _head.store(node);
        _tail.store(node);
    }
    LockFreeQueue(const LockFreeQueue&) = delete;
    LockFreeQueue& operator=(const LockFreeQueue&) = delete;
    
    void Push(const T& value)
    {
        std::unique_ptr<T> newData = std::make_unique<T>(value);

        CountedNodePtr dummy;
        dummy.ptr = new Node;
        dummy.externalCount = 1;

        CountedNodePtr oldTail = _tail.load();
        while (true) { 
            IncreaseExternalCount(_tail, oldTail);

            T* oldData = nullptr;
            if (oldTail.ptr->data.compare_exchange_strong(oldData, newData.get())) {
                oldTail.ptr->next = dummy;
                oldTail = _tail.exchange(dummy);

                FreeExternalCount(oldTail);
                newData.release();
                break;
            }

            oldTail.ptr->ReleaseRef();
        }
    }

    std::shared_ptr<T> TryPop()
    {
        CountedNodePtr oldHead = _head.load();

        while (true) {
            IncreaseExternalCount(_head, oldHead);
            
            Node* ptr = oldHead.ptr;
            if (ptr == _tail.load().ptr) {
                ptr->ReleaseRef();
                return std::shared_ptr<T>();
            }

            if (_head.compare_exchange_strong(oldHead, ptr->next)) {
                T* res = ptr->data.load(); // exchange(nullptr);
                FreeExternalCount(oldHead);
                return std::shared_ptr<T>(res);
            }

            ptr->ReleaseRef();
        }
    }

private:
    static void IncreaseExternalCount(std::atomic<CountedNodePtr>& counter, CountedNodePtr& oldCounter)
    {
        while (true) {
            CountedNodePtr newCounter = oldCounter;
            newCounter.externalCount++;

            if (counter.compare_exchange_strong(oldCounter, newCounter)) {
                oldCounter.externalCount = newCounter.externalCount;
                break;
            }
        }
    }

    static void FreeExternalCount(CountedNodePtr& oldNodePtr)
    {
        Node* ptr = oldNodePtr.ptr;
        const int32 countIncrease = oldNodePtr.externalCount - 2;

        NodeCounter oldCounter = ptr->count.load();
        while (true) {
            NodeCounter newCounter = oldCounter;
            newCounter.externalCountRemaining--;
            newCounter.internalCount += countIncrease;

            if (ptr->count.compare_exchange_strong(oldCounter, newCounter)) {
                if (newCounter.internalCount == 0 && newCounter.externalCountRemaining == 0) {
                    delete ptr;
                }
                
                break;
            }
        }
    }

private:
    std::atomic<CountedNodePtr> _head;
    std::atomic<CountedNodePtr> _tail;
};
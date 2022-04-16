#include "pch.h"
#include "CorePch.h"
#include "CoreMacro.h"
#include "ThreadManager.h"
#include <atomic>
#include <mutex>

class TestLock {
    //Lock _lock;
    USE_LOCK;

public:
    int32 TestRead()
    {
        //ReadLockGuard lockGuard(_lock);
        READ_LOCK;

        if (_queue.empty())
            return -1;

        return _queue.front();
    }

    void TestPush()
    {
        //WriteLockGuard lockGuard(_lock);

        WRITE_LOCK;
        _queue.push(std::rand() % 100);
    }

    void TestPop()
    {
        //WriteLockGuard lockGuard(_lock);
        WRITE_LOCK;

        if (!_queue.empty())
            _queue.pop();
    }

private:
    std::queue<int32> _queue;
};

TestLock testLock;

void ThreadWrite()
{
    while (true)
    {
        testLock.TestPush();
        std::this_thread::sleep_for(1ms);
        testLock.TestPop();
    }
}

void ThreadRead()
{
    while (true) { 
        int32 value = testLock.TestRead();
        std::cout << value << "\n";
        std::this_thread::sleep_for(1ms);
    }
}

int main()
{
    //for (int32 i = 0; i < 1; ++i)
        GThreadManager->Launch(ThreadWrite);

    //for (int32 i = 0; i < 1; ++i)
        GThreadManager->Launch(ThreadRead);

    GThreadManager->Join();
}
#include "pch.h"
#include "ThreadManager.h"
#include "CoreTLS.h"
#include "CoreGlobal.h"

ThreadManager::ThreadManager()
{
	// Main Thread
    InitTLS();
}

ThreadManager::~ThreadManager()
{
    Join();
}

void ThreadManager::Launch(std::function<void(void)> callback)
{
    LockGuard guard(_lock);

    _threads.push_back(std::thread([=]() {
        InitTLS();
        callback();
        DestroyTLS();
    }));
}

void ThreadManager::Join()
{
    for (std::thread& t : _threads) {
        if (t.joinable())
            t.join();
    }
    _threads.clear();
}

void ThreadManager::InitTLS()
{
    static Atomic<uint32> SThreadId = 1;
    LThreadId = SThreadId.fetch_add(1);
}

void ThreadManager::DestroyTLS()
{

}

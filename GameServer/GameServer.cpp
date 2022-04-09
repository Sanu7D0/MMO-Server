#include "pch.h"
#include "CorePch.h"
#include "CoreMacro.h"
#include "ThreadManager.h"
#include <atomic>
#include <mutex>

using namespace std::chrono_literals;

CoreGlobal Core;

void ThreadMain()
{
    while (true) {
        std::cout << "Hi im thread " << LThreadId << "\n";
        std::this_thread::sleep_for(1s);
    }
}

int main()
{
    for (int32 i = 0; i < 5; ++i)
        GThreadManager->Launch(ThreadMain);

    GThreadManager->Join();
}
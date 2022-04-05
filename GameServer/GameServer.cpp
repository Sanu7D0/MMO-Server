#include "pch.h"
#include "CorePch.h"

#include <atomic>
#include <mutex>

#include "ConcurrentQueue.h"
#include "ConcurrentStack.h"
using namespace std::chrono_literals;

LockQueue<int32> q;
LockFreeStack<int32> s;

void Push()
{
    while (true) {
        int32 val = rand() % 100;
        s.Push(val);

        std::this_thread::sleep_for(10ms);
    }
}

void Pop()
{
    while (true) {

        int32 data = 0;
        if (s.TryPop(data))
            cout << data << "\n";
    }
}

int main()
{
    std::thread t1(Push);
    std::thread t2(Pop);
    std::thread t3(Pop);

    t1.join();
    t2.join();
    t3.join();
}
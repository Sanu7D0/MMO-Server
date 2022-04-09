#pragma once

#include <thread>
#include <functional>

class ThreadManager {
public:
    ThreadManager();
    ~ThreadManager();

    void Launch(std::function<void(void)> callback);
    void Join();

    static void InitTLS();
    static void DestroyTLS();

private:
    Mutex _lock;
    std::vector<std::thread> _threads;
};

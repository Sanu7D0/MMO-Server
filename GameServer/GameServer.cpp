#include "pch.h"
#include "CorePch.h"
#include "CoreMacro.h"
#include "ThreadManager.h"
#include <atomic>
#include <mutex>

#include "PlayerManager.h"
#include "AccountManager.h"

int main()
{
    GThreadManager->Launch([=] {
        while (true) {
            std::cout << "PlayerThenAccount\n";
            GPlayerManager.PlayerThenAaccount();
            std::this_thread::sleep_for(100ms);
        }
    });
    GThreadManager->Launch([=] {
        while (true) {
            std::cout << "AccountThenPlayer\n";
            GAccountManager.AccountThenPlayer();
            std::this_thread::sleep_for(100ms);
        }
    });

    GThreadManager->Join();
}
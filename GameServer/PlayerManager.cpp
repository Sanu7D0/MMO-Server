#include "pch.h"
#include "PlayerManager.h"
#include "AccountManager.h"

PlayerManager GPlayerManager;

void PlayerManager::PlayerThenAaccount()
{
	WRITE_LOCK;
	std::this_thread::sleep_for(1s);
    GAccountManager.Lock();
}

void PlayerManager::Lock()
{
	WRITE_LOCK;
}

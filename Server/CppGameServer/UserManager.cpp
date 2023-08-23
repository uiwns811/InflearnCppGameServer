#include "pch.h"
#include "UserManager.h"
#include "AccountManager.h"

void UserManager::ProcessSave()
{
	// AccountLock : guard로 lock한 내부에서 또 잡음
	Account* account = AccountManager::Instance()->GetAccount(100);

	// userLock
	lock_guard<mutex> guard(_mutex);

	// TODO
}
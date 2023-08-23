#include "pch.h"
#include "AccountManager.h"
#include "UserManager.h"

void AccountManager::ProcessLogin()
{
	// Accountlock
	lock_guard<mutex> guard(_mutex);

	// UserLock : guard�� lock�� ���ο��� �� ����
	User* user = UserManager::Instance()->GetUser(100);
}

// ��Ͷ� �ذ� ���
// 1. Lock�� ������ �����Ѵ�
// 2. Mutex�� �����ϴ� Ŭ������ ����� id�� �ο��� ��, �� ū �ְ� ������ ���� ����ǵ��� �ϴ� �ļ� !!����
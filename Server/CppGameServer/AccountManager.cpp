#include "pch.h"
#include "AccountManager.h"
#include "UserManager.h"

void AccountManager::ProcessLogin()
{
	// Accountlock
	lock_guard<mutex> guard(_mutex);

	// UserLock : guard로 lock한 내부에서 또 잡음
	User* user = UserManager::Instance()->GetUser(100);
}

// 재귀락 해결 방법
// 1. Lock의 순서를 보장한다
// 2. Mutex를 관리하는 클래스를 만들어 id를 부여한 뒤, 더 큰 애가 무조건 먼저 실행되도록 하는 꼼수 !!ㅈㅂ
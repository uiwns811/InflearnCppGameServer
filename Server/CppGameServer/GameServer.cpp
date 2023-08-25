#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex> 
#include <Windows.h>
using namespace std;

mutex m;
queue<int32> q;
HANDLE handle;

void Producer()
{
	while (true) {
		{
			unique_lock<mutex> lock(m);
			q.push(100);
		}

		::SetEvent(handle);				// 커널 오브젝트를 Signal 상태로 바꿔준다
		this_thread::sleep_for(100ms);
	}
}

void Consumer()
{
	while (true) {
		::WaitForSingleObject(handle, INFINITE);
		// 커널 오브젝트가 Signal : 그대로, Non-Signal : 이 스레드는 대기함.
		// ManualReset == false : 여기서 Non-Signal 상태가 됨
		// else
		// ::ResetEvent(handle);


		// 무한루프 돌면서 무의미한 실행하는 것을 막아준다.

		unique_lock<mutex> lock(m);
		if (!q.empty()) {
			int32 data = q.front();
			q.pop();
			cout << data << endl;
		}
	}
}

int main()
{
	// 커널 오브젝트 : 커널에서 관리하는 객체
	// Usage Count : 이 오브젝트를 몇 명이 사용하고 있는지
	// Signal (파란불) / Non-Signal (빨간불) >> boolean
	// Auto / Manual  >> boolean

	handle = ::CreateEvent(NULL/*보안속성*/, FALSE/*Manual Reset : True - 수동, False - 자동*/, FALSE /*bInitialState*/, NULL);

	thread t1(Producer);
	thread t2(Consumer);
	
	t1.join();
	t2.join();

	::CloseHandle(handle);
}

// 커널 오브젝트를 통해 프로세스 간의 동기화를 구현할 수 있다.

// SpinLock : 유저레벨에서 일어나는 동기화
// Event : 커널단계까지 개입하는 동기화 (활용성이 높지만 비용이 높다)
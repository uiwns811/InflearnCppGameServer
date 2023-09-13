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

// [참고] CV : User-Level Object (커널 오브젝트 X)
condition_variable cv;
// mutex와 짝지어서 동작
// event와 유사
 

//#include <condition_variable>
//condition_variable_any cv;

void Producer()
{
	while (true) 
	{
		// 1. Lock을 잡고
		// 2. 공유 변수 값 수정
		// 3. Lock을 풀고
		// 4. 조건변수를 통해 다른 쓰레드에게 통지

		{
			unique_lock<mutex> lock(m);
			q.push(100);
		}

		cv.notify_one();		// wait중인 쓰레드가 있으면 딱 1개를 깨운다.
		
		//this_thread::sleep_for(100ms);
	}
}

void Consumer()
{
	while (true) {
		::WaitForSingleObject(handle, INFINITE);

		// 이 상황에서 WaitForSingleObject와 lock 잡는 부분 사이에서 다른 애가 lock을 잡을 수 있음

		unique_lock<mutex> lock(m);
		if (!q.empty()) {
			int32 data = q.front();
			q.pop();
			cout << q.size() << endl;
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
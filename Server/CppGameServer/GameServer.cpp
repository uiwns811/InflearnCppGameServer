#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>

// __declspec(thread) int32 value;
thread_local int32 LThreadId = 0;
int32 gValue;

void ThreadMain(int32 threadId)
{
	LThreadId = threadId;
	
	while (true) {
		cout << "Hi ! I am thread " << LThreadId << endl;
		this_thread::sleep_for(1s);
	}
}

int main()
{
	vector<thread> threads;
	
	for (int32 i = 0; i < 10; i++) {
		int32 threadId = i + 1;
		threads.push_back(thread(ThreadMain, threadId));
	}

	for (thread& t : threads)
		t.join();
}

// TLS (Thread Local Storage)
// 모든 쓰레가 공유하는 공간 : Heap, Data
// 각 쓰레드 고유의 공간 : stack, TLS
// 자기가 사용할 거를 TLS에 가져오는 동안 lock걸면 그 이후에는 경합 X

// TLS vs Stack
// stack : 불안정한 공간
// TLS : 전역! "나만의" 전역 공간

// 매 번 lock걸고 접근하지 말고, 큰 덩어리를 TLS로 가져와서 거기서 꺼내 쓰자!
// 쓰레드 별로 들어있어야 하는 정보들을 TLS에 넣어놓고 사용하자

// sendbuf도 각 TLS에 넣어놓고 꺼내서 쓰자!


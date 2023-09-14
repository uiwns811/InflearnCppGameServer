#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>

atomic<bool> ready;
int32 value;

void Producer()
{
	value = 10;

	ready.store(true, memory_order::memory_order_release);
	//------------------절취선-----------------
	// 위 명령이 아래로 내려갈 수 없다.
	// 이 선 기준으로 앞뒤 순서를 바꿀 수 없는거지, 위에서 끼리끼리나 아래서 끼리끼리는 바뀔 수 있음
}

void Consumer()
{
	//------------------절취선-----------------
	while (ready.load(memory_order::memory_order_acquire) == false)
		;

	cout << value << endl;
}

int main()
{
	ready = false;
	value = 0;
	thread t1(Producer);
	thread t2(Consumer);
	t1.join();
	t2.join();

	// Memory Model (정책)
	// 1) Sequentially Consistent (seq_cst)
	// 2) Acquire-Release (acquire, release)
	// 3) Relaxed (relaxed)
	
	// 1) seq_cst (가장 엄격 : 컴파일러 최적화 여지 적음 = 직관적)
	// - 가시성 문제, 코드 가시성 문제 바로 해결 !
	// - 결과 : 10
	// 
	// 2) acquire_release
	// 중간 단계
	// release 명령 이전의 메모리 명령들이 해당 명령 이후로 재배치 되는 것을 금지
	// = 조건부 금지
	// acqure로 같은 변수를 읽는 쓰레드가 있다면
	// release 이전 명령들이 acquire하는 순간에 접근 가능하다 (가시성 문제 해결!)
	// - value가 100% 10으로 나옴 (보장됨)
	// 
	// 3) relaxed (자유롭다 : 컴파일러 최적화 여지 높음 = 직관적X)
	// 너무나도 자유롭다 - 코드 재배치도 멋대로 가능! 가시성 문제 해결X
	// 가장 기본 조건 : 동일 객체에 대한 동일 관전 순서만 보장
	// - 거의 사용하지 않는다.


	// Intel, AMD의 경우 애당초 순차적 일관성 보장!
	// seq_cst 버전을 써도 별다른 부하가 없음
	// ARM은 의미 있음.

	// 메모리 베리어는 CPU에서 지원
	// C++ 표준에서도 만들 수 있음
	std::atomic_thread_fence(memory_order::memory_order_relaxed);
	// Fence : 메모리 가시성 강제 + 메모리 재배치 금지
}
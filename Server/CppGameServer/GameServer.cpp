#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>

atomic<bool> flag = false;

int main()
{

	//flag = true;
	flag.store(true, memory_order::memory_order_seq_cst);

	//bool val = flag;
	bool val = flag.load(memory_order::memory_order_seq_cst);

	cout << flag.is_lock_free() << endl;
	// 1 == true == lock이 없다.
	// -> 이미 CPU 내부적으로 Lock을 걸 필요가 없는 atomic한 연산이다.

	// 이전 flag값을 prev에 넣고 flag 값을 수정
	{
		// bool prev = flag;
		// flag = true;
		bool prev = flag.exchange(true);
		// -> flag의 값을 true로 변경하고, 변경 이전의 값을 리턴.
		// 위의 두 연산을 원자적으로 실행하는 함수
	}

	// CAS (Compare-And-Swap) 조건부 수정
	{
		bool expected = false;
		bool desired = true;
		flag.compare_exchange_strong(expected, desired);

		// Spurious Failure (가짜실패)
		if (flag == expected) {
			// 다른 쓰레드의 방해를 받아  interruption을 받아 중간에 실패할 수 있음
			// if (묘한 상황) return false;

			// expected = flag;
			flag = desired;
			return true;
		}
		else {
			expected = flag;
			return false;
		}
		// compare_exchange_strong : 위 연산을 원자적으로 실행해준다.
		// expected와 같으면 desired로 수정해주고 true를 리턴
		// expected와 같지 않으면 expected에 flag값을 넣어주고 false 리턴
	}

	bool expected = false;
	bool desired = true;
	flag.compare_exchange_weak(expected, desired);
	// weak : 동작 자체는 동일하지만, 위에 적어놓은 Spurious Failure로 인해 중간에 실패하는 상황이 생길 수 있음
	// strong : 한바퀴를 더 돌아 성공할 때까지 도전

	// weak를 사용할거면 while루프를 해주는게 일반적.
	// 성능 차이가 일반적으로 크게 없다.
}

// 캐시와 CPU 내부 최적화 작업으로 인해 멀티쓰레드 환경에서는 문제가 발생할 수 있다.

// 메모리모델 (Memory Model)
// 
// # Atomic (원자적) 연산 : CPU가 한 번에 처리할 수 있는 연산
// 여러 단계 쪼개서 해야 하면 원자적이지 않음
//
// # Atomic 연산에 한해, 모든 쓰레드가 동일 객체에 대해 동일한 수정 순서를 관찰
// - **동일 객체** 에 대해 **동일 순서**가 중요!!
// == 과거로는 갈 수 없다. (절대 시간 순서를 반드시 지켜야 한다)
// 
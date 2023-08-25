#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex> 
using namespace std;

class SpinLock
{
public:
	void lock()
	{
		// CAS (Compare-And-Swap)
		bool expected = false;
		bool desired = true;

		while (_locked.compare_exchange_strong(expected, desired) == false)
		{
			expected = false;

			//this_thread::sleep_for(std::chrono::milliseconds(100));
			this_thread::sleep_for(100ms);		
			this_thread::yield();				// 자기가 부여받은 time slice를 양보하고 커널모드로 돌아감
			// yield == sleep_for(0)

			// sleep_for : 인자 시간 동안 재스케쥴링 되지 않는다.
			// yield : 언제든지 다시 스케쥴링 될 수 있지만, 현재 time slice는 반환하겠다.
		}
	}

	void unlock()
	{
		_locked.store(false);
	}

private:
	atomic<bool> _locked = false;
};

int32 sum = 0;
mutex m;
SpinLock spinlock;

void Add()
{
	for (int32 i = 0; i < 100'000; i++)
	{
		lock_guard<SpinLock> guard(spinlock);
		sum++;
	}
}

void Sub()
{
	for (int32 i = 0; i < 100'000; i++)
	{
		lock_guard<SpinLock> guard(spinlock);
		sum--;
	}
}

int main()
{
	thread t1(Add);
	thread t2(Sub);
	t1.join();
	t2.join();

	cout << sum << endl;
}

// 커널 : 운영체제를 돌리기 위해 필요한 코드들이 위치한 영역 
// - (Window핵심 프로그램을 실행하기 위한 공간 : 관리자 모드)

// 스케줄러가 프로그램을 실행할 때, Time Slice(유효한 시간 동안 실행 보장)을 부여한다.
// Time Slice동안 실행했으면, 자발적으로 실행 소유권을 커널에게 넘겨줌
// Time Slice를 100% 실행해야 하는 것은 아님. 스스로 반환해도 되고, 시스템콜이 많아도 중간에 반환할 수 있음

// 시스템 콜(System Call) : 커널에 요청을 보냄
// - cout하면 시스템 콜 발생 -> 커널이 요청받은 일을 하고 다시 쓰레드로 돌아옴

// Sleep : 부여받은 TimeSlice를 반환하고 커널모드로 돌아감
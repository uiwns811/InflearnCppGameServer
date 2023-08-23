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

		// CAS 의사코드
		/*if (_locked == expected) {
			_locked = desired;
			return true;
		}
		else {
			expected = _locked;
			return false;
		}*/

		while (_locked.compare_exchange_strong(expected, desired) == false)
		{
			expected = false;
		}

		//while (_locked)
		//{
		//	
		//}
		//_locked = true;
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

// volatile : 컴파일러에게 최적화하지 말아달라고 요청
// - C# : 메모리베리어, 가시성 기능 추가
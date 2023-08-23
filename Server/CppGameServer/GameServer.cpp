#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>

vector<int32> v;
mutex m;

template<typename T>
class LockGuard
{
public:
	LockGuard(T& m) {
		_mutex = &m;
		_mutex->lock();
	}

	~LockGuard()
	{
		_mutex->unlock();
	}
private:
	T* _mutex;
};

void Push()
{
	for (int32 i = 0; i < 10000; i++)
	{
		// 자물쇠 잠그기
		// m.lock();
		// lock_guard<mutex> lockGuard(m);
		// unique_lock<mutex> uniqueLock(m, defer_lock);
		// uniqueLock.lock();		// 잠기는 시점을 미룰 수 있다.

		v.push_back(i);

		if (i == 5000) {
			// m.unlock();
			break;
		}
		
		// 자물쇠 풀기
		// m.unlock();
	}
}

int main()
{
	// v.reserve(20000);
	thread t1(Push);
	thread t2(Push);

	t1.join();
	t2.join();

	cout << v.size() << endl;
}

// vector : push_back 할 때 잔여 메모리가 없으면 새로 할당받아서 기존 원소들의 내용을 복사하고 기존 메모리를 해제함
// 이러한 상황에서 두 스레드가 v에 동시에 접근하게 되면, t1에 의해 해제된 기존 v의 메모리에 t2가 접근하게 되는 문제가 발생할 수 있음.
// 또한, 두 스레드가 동시에 삽입한다면 같은 위치에 데이터가 두 번 기입되어 일부 데이터가 분실될 수 있음 (v.size() != 20000)
// - 이러한 자료구조는 멀티스레드에 안전하지 않음.

// Lock
// : 먼저 들어갈 애가 Lock하면, 다른 애는 접근한 애가 Unlock할 때까지 접근할 수 없음
// Mutual Exclusive (상호배타적)
// but 개오래걸림

// Lock 주의할 점
// 1. Lock을 재귀적으로 걸 수 있냐? (웬만하면 허용하는게 편함)
// - Mutex (X), Recursive Mutex (o)
// 2. Unlock하지 않는 경우
// - 프로그램이 영 영 끝나지 않음 ..
// - RAII 패턴 (Resource Acquisition Is Initialization) 활용
// - : 클래스를 이용해서 자동으로 lock, unlock 관리
// Mutex 쌩으로 하는 것보다는 lockguard를 사용ㅎ아자.
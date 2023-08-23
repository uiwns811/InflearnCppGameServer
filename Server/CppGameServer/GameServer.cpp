#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>

atomic<int32> sum = 0;

void Add()
{
	for (int32 i = 0; i < 1'000'000; i++)
		//sum++;
		sum.fetch_add(1);

	// 디스어셈블리에서 확인한 sum++의 매커니즘
	// int32 eax = sum;
	// eax = eax + 1;
	// sum = eax;
}

void Sub()
{
	for (int32 i = 0; i < 1'000'000; i++)
		//sum--;
		sum.fetch_add(-1);
}

int main()
{
	Add();
	Sub();
	cout << sum << endl;

	thread t1(Add);
	thread t2(Sub);
	t1.join();
	t2.join();
	cout << sum << endl;
}

// 멀티스레드 환경에서 데이터를 동시에 접근하게 될 때 문제가 생길 수 있다
// 위에 기술한 sum++의 3개의 과정은 명령어의 실행이 Atomic하다고 보장할 수 없음

// Atomic : All or Nothing (실행이 되거나 아예 안되거나)

// Atomic Class로 감싸주면 3단계의 명령어 실행이 한 번에 수행된다.
// - Atomic한 실행이 보장된다 !

// 둘 중 먼저 접근한 애가 먼저 수행하고,
// 그 아이의 수행이 완료될 때까지 CPU가 접근할 수 없도록 막아서 다른 애는 대기
#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>

int32 x = 0;
int32 y = 0;
int32 r1 = 0;
int32 r2 = 0;

volatile bool ready;

void Thread_1()
{
	while (!ready)
		;
	y = 1;		// Store y
	r1 = x;		// Load x
}

void Thread_2()
{
	while (!ready)
		;
	x = 1;		// store x
	r2 = y;		// load y
}

int main()
{
	int32 count = 0;
	
	while (true) {
		ready = false;
		count++;

		x = y = r1 = r2 = 0;

		thread t1(Thread_1);
		thread t2(Thread_2);

		ready = true;
		
		t1.join();
		t2.join();

		if (r1 == 0 && r2 == 0)
			break;
	}

	cout << count << "번 만에 빠져나옴" << endl;
}

// 83973번 만에 빠져나옴
// 거의 동시에 실행되는데 둘 중 먼저 실행되는 애가 있음. 그러면 어쨌든 r1이나 r2나 변경이 될 거임
// but 멀티쓰레드 환경에서는 이러한 오류 (r1도 r2도 0)가 생길 수 있음

// 왜?
// 1. 가시성
// 캐시 : CPU가 쓰거나 읽을 값이 캐시에 있으면 RAM까지 가지 않고 캐시에서 값을 가져옴
// - CPU의 코어마다 각각의 캐시를 가지고 있음.
// 읽어온 값, 쓰는 값이 실제 메모리에 쓰지 않고, 캐시에만 썼을 수도 있음
// 단일쓰레드라면 내가 사용하던 캐시의 값에 적용되어 있으니 문제 없지만,
// 멀티쓰레드에서는 내 캐시에 적용되지 못했고, 램에도 없음
// == 가시성이 보장되지 않는다.
// 
// 2. 코드 재배치
// 컴파일러가 내가 만든 코드를 기계어로 변환 -> CPU가 기계어 실행
// 컴파일러는 100% 그대로 번역하지 않을 수도 있다.
// 검수 중 효율성을 위해 코드 순서를 변경할 수 있다.
// 컴파일러는 멀티쓰레드 환경을 고려하지 않기 때문이다.

// -------------------------------------------
// CPU는 명령어를 실행할 때 CPU 파이프라인(4가지 단계)을 거친다.
// 1. Fetch : 명령어 가져오기
// 2. Decode : 해독
// 3. Execute : 실행
// 4. Write-back : 결과를 다시 가져다줌
// 
// 명령어를 여러 개 요청했을 때,
// 경우에 따라 순서를 바꿔서 수행하는게 더 효율적이라면 CPU가 알아서 순서를 변경해줄 수 있음
// 단일 쓰레드 기준으로는 전혀 문제가 없으나, 멀티쓰레드에서는 로직이 꼬일 수 있음.

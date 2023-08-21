#include "pch.h"
#include <iostream>
#include "CorePch.h"

#include <thread>
// - 윈도우, 리눅스 관계없이 범용적인 코드 작성 가능

// System Call (OS 커널에 요청 후 OS가 처리)
// - 스레드 생성

void HelloThread()
{
	cout << "Hello Thread~" << endl;
}

int main()
{
	thread t;					// 생성
	auto id1 = t.get_id();			// 쓰레드 고유의 id	
	
	t = thread(HelloThread);	// 실행

	int32 count = t.hardware_concurrency();		// CPU 코어 개수
	// - 논리적으로 실행할 수 있는 프로세스의 개수 (0을 리턴하기도 함)
	auto id2 = t.get_id();			// 쓰레드 고유의 id	

	// t.detach();			
	// 쓰레드 객체 t에서 실제 쓰레드를 분리
	// - t를 이용해 만든 정보를 추출할 수 없음 : 활용할 일이 없음

	if (t.joinable()) t.join();
	// 연결된 쓰레드가 있는지 여부 확인

	//t.join();			// 실질적으로 구동된 쓰레드에서 쓰레드 t의 종료를 기다림
	cout << "Hello Main" << endl;
}
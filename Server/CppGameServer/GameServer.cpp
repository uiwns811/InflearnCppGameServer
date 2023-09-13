#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <future>
using namespace std;

int64 result;

int64 Calculate()
{
	int64 sum = 0;

	for (int32 i = 0; i < 100'000; i++)
		sum += i;

	return sum;
}

void PromiseWorker(std::promise<string>&& promise)
{
	promise.set_value("Secret Message");
}

void TaskWorker(std::packaged_task<int64(void)>&& task)
{
	task();
}

int main()
{
	// 동기 (Synchronous) 실행
	//int64 sum = Calculate();
	//cout << sum << endl;

	// std::future : 단기 알바
	{
		// 옵션
		// 1. deferred : 지연해서 실행하세요
		// 2. async : 별도의 쓰레드를 만들어서 실행하세요
		// 3. deferred | async : 둘 중 알아서 골라주세요

		std::future<int64> future = std::async(std::launch::async, Calculate);
		// async : Calculate의 완료 여부에 상관없이 리턴함

		int sum = future.get();		// 결과물이 이제서야 필요하다!

		//std::future_status status = future.wait_for(1ms);		// n초동안 기다려주세요
		//if (status == future_status::ready) {
		//	// 일감 완료 : 여기서 처리
		//}

		//future.wait();				// wait_for 무한정

		// 동기 : 호출하는 순간 실행
		// 비동기 : 실행 시점이 뒤로 밀린다.

		// async : 사실상 멀티쓰레드 환경. (별도의 쓰레드가 Calculate를 병렬로 실행해줌)
		// deferred : 멀티쓰레드는 아님

		class Knight {
		public:
			int64 GetHp() { return 100; }
		};

		Knight knight;
		std::future<int64> future2 = std::async(std::launch::async, &Knight::GetHp, knight);
	}

	// std::promise (future 객체를 만들어주는 두 번째 방법)
	{
		// 미래(std::future)에 결과물을 반환해줄거라 약속 (std::promise)
		// promise는 다른 쓰레드에게 넘기고, future는 우리가 갖고 있을 거임
		std::promise<string> promise;
		std::future<string> future = promise.get_future();

		thread t(PromiseWorker, std::move(promise));
		string message = future.get();
		cout << message << endl;
		t.join();	
	}

	// std::packaged_task
	{
		// task : Calculate를 다른 쓰레드에서 호출해주세요~ 라고 일감 단위로 만들어 줌
		std::packaged_task<int64(void)> task(Calculate);
		std::future<int64> future = task.get_future();

		// Calculate의 결과를 future로 받아올 수 있다

		std::thread t(TaskWorker, std::move(task));
		int64 sum = future.get();
		cout << sum << endl;
		t.join();
	}
}

// 잠깐 기능을 사용하기 위해 쓰레드 관리를 하기보다는 future를 사용하자.

// future : 언젠가 미래에 결과물을 뱉어줄거야! (당장은 없을 수도 있지만 get하는 순간에는 결과물을 얻어올거야)


// 결론
// mutex, condition_variable까지 가지 않고 단순한 애들을 처리할 수 있는 것들
// - 일회성 이벤트에 유용 ! (무한루프 ㄴㄴ 한 번만 해야 하는 것)

// async : Calculate 처리를 위한 전용 쓰레드를 새로 만들어서 실행
// Task : 이미 존재하는 쓰레드에게 일감(Task)을 던져주고 실행하도록 함. 결과물은 future로 받음

// 1. async : 원하는 함수를 비동기적으로 실행
// 2. promise : 결과물을 promise를 통해 future로 받아줌
// 3. packaged_task : 원하는 함수의 실행 결과를 packaged_task를 통해 future로 받아줌
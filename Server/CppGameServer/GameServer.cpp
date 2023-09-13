#include "pch.h"
#include <iostream>
#include "CorePch.h"
#include <thread>
#include <atomic>
#include <mutex>
#include <Windows.h>

int32 buffer[10'000][10'000];

int main()
{
	memset(buffer, 0, sizeof(buffer));

	{
		uint64 start = GetTickCount64();

		int64 sum = 0;
		
		for (int32 i = 0; i < 10'000; i++) {
			for (int32 j = 0; j < 10'000; j++) {
				sum += buffer[i][j];
			}
		}

		uint64 end = GetTickCount64();
		cout << "Elapsed Tick : " << end - start << endl;
	}

	{
		uint64 start = GetTickCount64();

		int64 sum = 0;

		for (int32 i = 0; i < 10'000; i++) {
			for (int32 j = 0; j < 10'000; j++) {
				sum += buffer[j][i];
			}
		}

		uint64 end = GetTickCount64();
		cout << "Elapsed Tick : " << end - start << endl;
	}
}

// 결과
// Elapsed Tick : 172
// Elapsed Tick : 641

// 위 경우의 수가 캐시히트율이 높음 (인접한 메모리 공간을 접근함)
// 캐시미스 -> 램에 가서 데이터를 꺼내온다.
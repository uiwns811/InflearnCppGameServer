#pragma once

#include <mutex>

template <typename T>
class LockStack
{
public:
	LockStack() {};

	// 복사 불가능
	LockStack(const LockStack&) = delete;
	LockStack& operator=(const LockStack&) = delete;

	void Push(T value)
	{
		lock_guard<mutex> lock(m_mutex);
		m_stack.push(std::move(value));
		m_condVar.notify_one();
	}

	bool TryPop(T& value)
	{
		lock_guard<mutex> lock(m_mutex);
		if (m_stack.empty()) return false;

		// empty -> top -> pop을 TryPop 하나로
		value = std::move(m_stack.top());
		m_stack.pop();
		return true;
	}

	void WaitPop(T& value)
	{
		// condition_variable할 때는 내부적으로 lock을 사용해야 하므로 unique_lock 사용  
		unique_lock<mutex> lock;
		m_condVar.wait(lock, [this]() {return !m_stack.empty(); });
		value = std::move(m_stack.top());
		m_stack.pop();
	}

	// Empty를 체크하고 다른 연산을 하는건 의미가 없음 -> lock 2개로 쌓인 연산들이 atomic하지 않음
	bool Empty()
	{
		lock_guard<mutex> lock(m_mutex);
		return m_stack.empty();
	}

private:
	stack<T> m_stack;
	mutex m_mutex;
	condition_variable m_condVar;
};


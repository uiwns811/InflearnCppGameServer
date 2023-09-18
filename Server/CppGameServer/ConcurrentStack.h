#pragma once

#include <mutex>

template <typename T>
class LockStack
{
public:
	LockStack() {};

	// ���� �Ұ���
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

		// empty -> top -> pop�� TryPop �ϳ���
		value = std::move(m_stack.top());
		m_stack.pop();
		return true;
	}

	void WaitPop(T& value)
	{
		// condition_variable�� ���� ���������� lock�� ����ؾ� �ϹǷ� unique_lock ���  
		unique_lock<mutex> lock;
		m_condVar.wait(lock, [this]() {return !m_stack.empty(); });
		value = std::move(m_stack.top());
		m_stack.pop();
	}

	// Empty�� üũ�ϰ� �ٸ� ������ �ϴ°� �ǹ̰� ���� -> lock 2���� ���� ������� atomic���� ����
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


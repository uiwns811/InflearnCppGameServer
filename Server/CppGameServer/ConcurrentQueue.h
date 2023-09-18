#pragma once

#include <mutex>

template <typename T>
class LockQueue
{
public:
	LockQueue() {}

	LockQueue(const LockQueue&) = delete;
	LockQueue& operator=(const LockQueue&) = delete;

	void Push(T value)
	{
		lock_guard<mutex> lock(m_mutex);
		m_queue.push(std::move(value));
		m_condVar.notify_one();
	}

	bool TryPop(T& value)
	{
		lock_guard<mutex> lock(m_mutex);
		if (m_queue.empty()) return false;

		// empty -> top -> pop�� TryPop �ϳ���
		value = std::move(m_queue.front());
		m_queue.pop();
		return true;
	}

	void WaitPop(T& value)
	{
		// condition_variable�� ���� ���������� lock�� ����ؾ� �ϹǷ� unique_lock ���  
		unique_lock<mutex> lock;
		m_condVar.wait(lock, [this]() {return !m_queue.empty(); });
		value = std::move(m_queue.front());
		m_queue.pop();
	}

private:
	queue<T> m_queue;
	mutex m_mutex;
	condition_variable m_condVar;
};
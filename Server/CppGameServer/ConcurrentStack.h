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

template <typename T>
class LockFreeStack 
{
	struct Node
	{
		Node(const T& value) : data(make_shared<T>(value)), next(nullptr)
		{

		}

		shared_ptr<T> data;
		shared_ptr<Node> next;
	};

public:
	void Push(const T& value)
	{
		shared_ptr<Node> node = make_shared<Node>(value);
		node->next = atomic_load(&m_head);

		while (std::atomic_compare_exchange_weak(&m_head, &node->next, node) == false)
		{
		}
	}

	shared_ptr<T> TryPop()
	{
		shared_ptr<Node> oldHead = std::atomic_load(&m_head);

		while (oldHead && std::atomic_compare_exchange_weak(&m_head, &oldHead, oldHead->next) == false)
		{

		}

		if (oldHead == nullptr) return shared_ptr<T>();

		return oldHead->data;
	}

private:
	shared_ptr<Node> m_head;
};
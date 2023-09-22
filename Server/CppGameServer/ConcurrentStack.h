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
		Node(const T& value) : data(value)
		{

		}

		T data;
		Node* next;
	};

public:
	void Push(const T& value)
	{
		// 1) 새 노드를 만들고
		// 2) 새 노드의 next = head
		// 3) head = 새 노드
		Node* node = new Node(value);
		node->next = m_head;

 		/*if (m_head == node->next) {
			m_head = node;
			return true;
		}
		else {
			node->next = m_head;
			return false;
		}*/

		while (m_head.compare_exchange_weak(node->next, node) == false)
		{
			//node->next = m_head;
		}

		// 이 사이에 새치기 당하면?
		//m_head = node;
	}

	bool TryPop(T& value)
	{
		// 1) head 읽기
		// 2) head->next 읽기
		// 3) head = head->next
		// 4) data 추출해서 반환
		// 5) 추출한 노드 삭제

		Node* oldHead = m_head;
		
		/*if (m_head == oldHead) {
			m_head = oldHead->next;
			return true;
		}
		else {
			oldHead = m_head;
			return false;
		}*/
		while (oldHead && m_head.compare_exchange_weak(oldHead, oldHead->next) == false)
		{
			// oldHead = m_head;
		}

		if (oldHead == nullptr) return false;

		// Exception X
		value = oldHead->data;
		//delete oldHead;
		return true;
	}

private:
	atomic<Node*> m_head;
};
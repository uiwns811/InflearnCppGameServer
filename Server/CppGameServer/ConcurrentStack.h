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
		// 1) �� ��带 �����
		// 2) �� ����� next = head
		// 3) head = �� ���
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

		// �� ���̿� ��ġ�� ���ϸ�?
		//m_head = node;
	}

	bool TryPop(T& value)
	{
		// 1) head �б�
		// 2) head->next �б�
		// 3) head = head->next
		// 4) data �����ؼ� ��ȯ
		// 5) ������ ��� ����

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
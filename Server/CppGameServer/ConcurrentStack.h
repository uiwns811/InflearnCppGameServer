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
		Node(const T& value) : data(value), next(nullptr)
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

		while (m_head.compare_exchange_weak(node->next, node) == false)
		{
		}
	}

	bool TryPop(T& value)
	{
		// 1) head �б�
		// 2) head->next �б�
		// 3) head = head->next
		// 4) data �����ؼ� ��ȯ
		// 5) ������ ��� ����

		++m_popCount;

		Node* oldHead = m_head;
		
		while (oldHead && m_head.compare_exchange_weak(oldHead, oldHead->next) == false)
		{
		}

		if (oldHead == nullptr) {
			--m_popCount;
			return false;
		}

		value = oldHead->data;
		
		TryDelete(oldHead);
		//delete oldHead;
		// - ���ÿ� TryPop�� ��� �ش� �����͸� �����ؼ� cas���� ����ϰ� ���� ���� ����

		return true;
	}


	void TryDelete(Node* oldHead)
	{
		// �� �ܿ� ���� �ִ°�?
		if (m_popCount == 1)
		{
			// �� ȥ��

			// ���� ����� �ٸ� �����͵鵵 ����
			Node* node = m_pendingList.exchange(nullptr);

			if (--m_popCount == 0)
			{
				// �߰��� ����� �ְ� ���� -> ���� ����
				// ���� ����� �����ʹ� ������ �и��ص� ����.
				DeleteNode(node);
			}
			else if (node)
			{
				// ���� ���������� �ٽ� ���� ����
				ChainPendingNodeList(node);
			}
		
			// �� ������ ����
			delete oldHead;
		}
		else
		{
			// ���� ���� -> ���� ���ุ
			ChainPendingNode(oldHead);
			--m_popCount;
		}
	}

	void ChainPendingNodeList(Node* first, Node* last)
	{
		last->next = m_pendingList;

		while (m_pendingList.compare_exchange_weak(last->next, first) == false) 
		{
		}
	}

	void ChainPendingNodeList(Node* node)
	{
		Node* last = node;
		while (last->next) last = last->next;

		ChainPendingNodeList(node, last);
	}

	void ChainPendingNode(Node* node)
	{
		ChainPendingNodeList(node, node);
	}

	static void DeleteNode(Node* node)
	{
		while (node)
		{
			Node* next = node->next;
			delete node;
			node = next;
		}
	}

private:
	atomic<Node*> m_head;
	atomic<int32> m_popCount = 0;		// Pop�� �������� ������ ���� (TryPop ������ ������ +1, ���� �� -1)
	atomic<Node*> m_pendingList;		// ���� �Ǿ�� �� ���� (ù��° ��� : �ᱹ next�� �¹�����)
};
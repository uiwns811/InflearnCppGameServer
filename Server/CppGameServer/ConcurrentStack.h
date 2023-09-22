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
	struct Node;
	struct CountedNodePtr
	{
		int32 externalCount = 0;			// ��带 �����ϰ� �ִ� ���� ī��Ʈ
		Node* ptr = nullptr;
	};

	struct Node
	{
		Node(const T& value) : data(make_shared<T>(value))
		{

		}

		shared_ptr<T> data;
		atomic<int32> internalCount = 0;
		CountedNodePtr next;
	};

public:
	void Push(const T& value)
	{
		CountedNodePtr node;
		node.ptr = new Node(value);
		node.externalCount = 1;

		node.ptr->next = m_head;
		while (m_head.compare_exchange_weak(node.ptr->next, node) == false)
		{

		}
	}

	shared_ptr<T> TryPop()
	{
		CountedNodePtr oldHead = m_head;
		while (true)
		{
			// ������ ȹ�� (externalCount�� �� ���� ���� +1 �� �ְ� �̱�)
			IncreaseHeadCount(oldHead);
			// �ּ��� externalCount >= 2 -> ����X. (�����ϰ� ������ �� �ִ� ����)
			Node* ptr = oldHead.ptr;

			if (ptr == nullptr) return share_ptr<T>();

			// ������ ȹ�� (ptr->next�� head �ٲ�ġ���� �ְ� �̱�)
			if (m_head.compare_exchange_strong(oldHead, ptr->next))
			{
				shared_ptr<T> res;
				res->swap(ptr->data);

				// externalCount : ���۰� 1 (��ȣǥ : ������)
				// internalCount : ���۰� 0

				// external : 1 -> 2 (��+1) -> 4(��+1 ��+2)
				// internal : 0

				// �� ���� �� ���� �ִ°�?
				const int32 countIncrease = oldHead.externalCount - 2;
				if (ptr->internalCount.fetch_add(countIncrease) == --countIncrease)
					delete ptr;

				return res;
			}
			else if (ptr->internalCount.fetch_sub(1) == 1)
			{
				// �������� ������� �������� ���� -> �޼����� ���� �Ѵ�
				delete ptr;
			}
		}
	}

private:
	void IncreaseHeadCount(CountedNodePtr& oldCounter)
	{
		while (true)
		{
			// counter = ptr + Ƚ��
			CountedNodePtr newCounter = oldCounter;
			newCounter.externalCount++;

			if (m_head.compare_exchange_strong(oldCounter, newCounter)) 
			{
				oldCounter.externalCount = newCounter.externalCount;
				break;
			}
		}
	}

private:
	atomic<CountedNodePtr> m_head;
};


// shared_ptr�� ����� LockFreeStack
/*
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
*/
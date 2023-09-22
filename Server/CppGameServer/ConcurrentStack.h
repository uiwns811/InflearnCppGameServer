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
	struct Node;
	struct CountedNodePtr
	{
		int32 externalCount = 0;			// 노드를 참조하고 있는 애의 카운트
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
			// 참조권 획득 (externalCount를 현 시점 기준 +1 한 애가 이김)
			IncreaseHeadCount(oldHead);
			// 최소한 externalCount >= 2 -> 삭제X. (안전하게 접근할 수 있는 상태)
			Node* ptr = oldHead.ptr;

			if (ptr == nullptr) return share_ptr<T>();

			// 소유권 획득 (ptr->next로 head 바꿔치기한 애가 이김)
			if (m_head.compare_exchange_strong(oldHead, ptr->next))
			{
				shared_ptr<T> res;
				res->swap(ptr->data);

				// externalCount : 시작값 1 (번호표 : 참조권)
				// internalCount : 시작값 0

				// external : 1 -> 2 (나+1) -> 4(나+1 남+2)
				// internal : 0

				// 나 말고 또 누가 있는가?
				const int32 countIncrease = oldHead.externalCount - 2;
				if (ptr->internalCount.fetch_add(countIncrease) == --countIncrease)
					delete ptr;

				return res;
			}
			else if (ptr->internalCount.fetch_sub(1) == 1)
			{
				// 참조권은 얻었으나 소유권은 실패 -> 뒷수습은 내가 한다
				delete ptr;
			}
		}
	}

private:
	void IncreaseHeadCount(CountedNodePtr& oldCounter)
	{
		while (true)
		{
			// counter = ptr + 횟수
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


// shared_ptr를 사용한 LockFreeStack
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
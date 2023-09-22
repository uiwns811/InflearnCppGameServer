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
		Node(const T& value) : data(value), next(nullptr)
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

		while (m_head.compare_exchange_weak(node->next, node) == false)
		{
		}
	}

	bool TryPop(T& value)
	{
		// 1) head 읽기
		// 2) head->next 읽기
		// 3) head = head->next
		// 4) data 추출해서 반환
		// 5) 추출한 노드 삭제

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
		// - 동시에 TryPop할 경우 해당 포인터를 참조해서 cas에서 사용하고 있을 수도 있음

		return true;
	}


	void TryDelete(Node* oldHead)
	{
		// 나 외에 누가 있는가?
		if (m_popCount == 1)
		{
			// 나 혼자

			// 삭제 예약된 다른 데이터들도 삭제
			Node* node = m_pendingList.exchange(nullptr);

			if (--m_popCount == 0)
			{
				// 중간에 끼어든 애가 없음 -> 삭제 가능
				// 누가 끼어들어도 데이터는 어차피 분리해둔 상태.
				DeleteNode(node);
			}
			else if (node)
			{
				// 누가 끼어들었으니 다시 갖다 놓자
				ChainPendingNodeList(node);
			}
		
			// 내 데이터 삭제
			delete oldHead;
		}
		else
		{
			// 누가 있음 -> 삭제 예약만
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
	atomic<int32> m_popCount = 0;		// Pop을 실행중인 쓰레드 개수 (TryPop 들어오는 순간에 +1, 나갈 때 -1)
	atomic<Node*> m_pendingList;		// 삭제 되어야 할 노드들 (첫번째 노드 : 결국 next로 맞물려짐)
};
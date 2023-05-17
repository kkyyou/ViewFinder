#include "CThreadPool.h"
#include <Windows.h>
#include <assert.h>

CThreadPool::CThreadPool() :
	m_threadSize(0),
	m_stopAll(false)
{
}

CThreadPool::CThreadPool(size_t threadSize) :
	m_threadSize(threadSize),
	m_stopAll(false)
{
}

CThreadPool::~CThreadPool()
{
	m_stopAll = true;
	m_cv.notify_all();
	for (auto& t : m_threads)
	{
		if (t.joinable())	
			t.join();
	}
}

void CThreadPool::EnqueueJob(std::function<void()> job)
{
	if (m_stopAll)
	{
		return;
	}

	{
		std::lock_guard<std::mutex> lock(m_mutex);
		m_jobs.push(std::move(job));
	}
	m_cv.notify_one();
}

void CThreadPool::InitThreadPool(size_t threadSize)
{
	if (!m_threads.empty())
	{
		assert(0);
		Stop();
	}

	m_threadSize = threadSize;
	m_stopAll = false;

	// 쓰레드 사이즈 할당 및 추가
	m_threads.reserve(threadSize);
	for (size_t i = 0; i < m_threadSize; i++)
	{
		m_threads.emplace_back([this]() { this->WorkerThread(); });
	}
}

void CThreadPool::Stop()
{
	{
		std::unique_lock<std::mutex> lock(m_mutex);
		m_stopAll = true;

		while (!m_jobs.empty())
		{
			m_jobs.pop();
		}
	}

	m_cv.notify_all();
	for (int i = 0; i < (int)m_threads.size(); i++)
	{
		if (m_threads[i].joinable())
			m_threads[i].join();
	}

	m_threads.clear();
}

bool CThreadPool::IsStop()
{
	return m_stopAll;
}

void CThreadPool::WorkerThread()
{
	while (true)
	{
		std::unique_lock<std::mutex> lock(m_mutex);

		// 현재 뮤텍스의 잠금을 풀고 notify_one, notify_all 기다린다.
		// 깨어나면 잠금을 다시 걸게된다.
		m_cv.wait(lock, [this] { return !(this->m_jobs.empty()) || m_stopAll; });
		if (m_stopAll && m_jobs.empty())
		{
			return;
		}

		// 큐에서 Job 가져오기
		std::function<void()> job = std::move(m_jobs.front());
		m_jobs.pop();
		lock.unlock();

		// Do Job
		job();
	}
}

#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <condition_variable>
#include <mutex>

class CThreadPool
{
public:
	CThreadPool();
	CThreadPool(size_t threadSize);
	~CThreadPool();

	void							  EnqueueJob(std::function<void()> job);
	void							  InitThreadPool(size_t threadSize);
	void							  Stop();
	bool							  IsStop();

private:							  
	void							  WorkerThread();
									  
private:							  
	size_t							  m_threadSize;
	std::vector<std::thread>		  m_threads;
	std::queue<std::function<void()>> m_jobs;
	std::condition_variable			  m_cv;
	std::mutex						  m_mutex;
	bool							  m_stopAll;
};


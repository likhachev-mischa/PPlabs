#include "ThreadPool.h"

ThreadPool::ThreadPool(): m_shouldTerminate(false), m_threadCount(0)
{
	start();
}

ThreadPool::~ThreadPool()
{
	stop();
}

void ThreadPool::start()
{
	m_threadCount = std::thread::hardware_concurrency();
	for (uint32_t i = 0; i < m_threadCount; ++i)
	{
		m_threads.emplace_back(&ThreadPool::threadLoop, this);
	}
}

void ThreadPool::threadLoop()
{
	while (true)
	{
		std::function<void()> job;
		{
			std::unique_lock<std::mutex> lock(m_queueMutex);
			m_mutexCondition.wait(lock, [this]
			{
				return !m_jobs.empty() || m_shouldTerminate;
			});
			if (m_shouldTerminate)
			{
				_endthread();
			}
			job = m_jobs.front();
			m_jobs.pop();
		}
		job();
	}
}

void ThreadPool::queueJob(const std::function<void()>& job)
{
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		m_jobs.push(job);
	}
	m_mutexCondition.notify_one();
}

bool ThreadPool::isBusy()
{
	bool isPoolBusy;
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		isPoolBusy = !m_jobs.empty();
	}
	return isPoolBusy;
}

uint32_t ThreadPool::getThreadCount()
{
	return m_threadCount;
}

void ThreadPool::stop()
{
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		m_shouldTerminate = true;
	}
	m_mutexCondition.notify_all();
	for (std::thread& activeThread : m_threads)
	{
		joinThread(activeThread);
	}
	m_threads.clear();
}

void ThreadPool::joinThread(std::thread& thread)
{
	if (thread.joinable())
	{
		thread.join();
	}
}

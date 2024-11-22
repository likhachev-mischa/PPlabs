#include "WindowsThreadPool.h"

#include <iostream>
#include <processthreadsapi.h>
#include <mutex>

WindowsThreadPool::WindowsThreadPool() : m_shouldTerminate(false), m_threadCount(0)
{
	start();
}

void WindowsThreadPool::queueJob(const std::function<void()>& job)
{
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		m_jobs.push(job);
	}
	m_mutexCondition.notify_one();
}

bool WindowsThreadPool::isBusy()
{
	bool isPoolBusy;
	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		isPoolBusy = !m_jobs.empty();
	}
	return isPoolBusy;
}

uint32_t WindowsThreadPool::getThreadCount()
{
	return m_threadCount;
}

WindowsThreadPool::~WindowsThreadPool()
{
	stop();
}

uint32_t WindowsThreadPool::threadLoop(void* param)
{
	WindowsThreadPool* pool = static_cast<WindowsThreadPool*>(param);
	while (true)
	{
		std::function<void()> job;
		{
			std::unique_lock<std::mutex> lock(pool->m_queueMutex); //lock_guard
			pool->m_mutexCondition.wait(lock, [pool]
			{
				return !pool->m_jobs.empty() || pool->m_shouldTerminate;
			});
			if (pool->m_shouldTerminate)
			{
			//	_endthreadex(0);
				return 0;
			}
			job = pool->m_jobs.front();
			pool->m_jobs.pop();
		}
		job();
	}
}

void WindowsThreadPool::start()
{
	m_threadCount = std::thread::hardware_concurrency();
	m_threads.reserve(m_threadCount);
	for (size_t i = 0; i < m_threadCount; ++i)
	{
		uintptr_t thread = _beginthreadex(nullptr, 148888, threadLoop, this, 0, nullptr);
		//	HANDLE thread = CreateThread(nullptr, 0, threadLoop, this, 0, nullptr);
		m_threads.push_back(thread);
	}
}

void WindowsThreadPool::stop()
{
	//{
	//	std::unique_lock<std::mutex> lock(m_queueMutex);
	//	m_shouldTerminate = true;
	//}
	//m_mutexCondition.notify_all();
	//for (HANDLE& activeThread : m_threads)
	//{
	//	joinThread(activeThread);
	//}
	//m_threads.clear();

	{
		std::unique_lock<std::mutex> lock(m_queueMutex);
		m_shouldTerminate = true;
	}
	m_mutexCondition.notify_all();

	for (uintptr_t thread : m_threads)
	{
		bool result =CloseHandle((HANDLE)thread);
		std::cout << "result: " << result << std::endl;
	}
}

void WindowsThreadPool::joinThread(HANDLE thread)
{
	std::thread* t = static_cast<std::thread*>(thread);
	if (t->joinable())
	{
		t->join();
	}
}

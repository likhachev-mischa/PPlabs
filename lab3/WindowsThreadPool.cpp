#include "WindowsThreadPool.hpp"

#include <iostream>
#include <mutex>

WindowsThreadPool::WindowsThreadPool() : m_threadCount(0)
{
	start();
}

void WindowsThreadPool::queueJob(const std::function<void()>& job)
{
	printf("job is queued\n");
	{
		std::lock_guard<std::mutex> lock(m_threadData.queueMutex);
		m_threadData.jobs.push(job);
	}
	m_threadData.mutexCondition.notify_one();
}

bool WindowsThreadPool::isBusy()
{
	bool isPoolBusy;
	{
		std::lock_guard<std::mutex> lock(m_threadData.queueMutex);
		isPoolBusy = !m_threadData.jobs.empty();
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

unsigned __stdcall WindowsThreadPool::threadLoop(void* param)
{
	auto data = static_cast<ThreadData*>(param);
	while (!data->shouldTerminate)
	{
		std::function<void()> job = nullptr;
		{
			std::unique_lock<std::mutex> lock(data->queueMutex);
			data->mutexCondition.wait(lock);

			if (!data->jobs.empty())
			{
				job = data->jobs.front();
				data->jobs.pop();
			}
		}
		if (job)
		{
			printf("%lu is doing it's job\n", GetCurrentThreadId());
			job();
			printf("%lu has done it's job\n", GetCurrentThreadId());
		}
	}
	printf("%lu is terminated\n", GetCurrentThreadId());
	_endthreadex(0);
	return 0;
}

void WindowsThreadPool::start()
{
	m_threadCount = std::thread::hardware_concurrency();
	printf("THREAD COUNT = %llu\n", m_threadCount);
	printf("THREAD ID | ACTION\n");
	m_threads.reserve(m_threadCount);

	std::lock_guard<std::mutex> lock(m_threadData.queueMutex);
	for (size_t i = 0; i < m_threadCount; ++i)
	{
		uint32_t threadId;
		uintptr_t thread = _beginthreadex(nullptr, 0, &threadLoop, &m_threadData, 0, &threadId);
		printf("%u is created \n", threadId);
		m_threads.push_back((HANDLE)(thread));
	}
}

void WindowsThreadPool::stop()
{
	{
		std::lock_guard<std::mutex> lock(m_threadData.queueMutex);
		m_threadData.shouldTerminate = true;
	}
	m_threadData.mutexCondition.notify_all();

	WaitForMultipleObjects(m_threadCount, m_threads.data(), TRUE, INFINITE);

	for (size_t i = 0; i < m_threadCount; ++i)
	{
		CloseHandle(m_threads[i]);
	}

	m_threads.clear();
}

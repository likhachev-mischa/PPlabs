#include "ThreadPool.hpp"

#include <iostream>
#include <mutex>
#include <thread>

#include "Platform.hpp"

uint32_t getThreadId()
{
#if defined (_WIN32) || defined (_WIN64)
	return GetCurrentThreadId();
#else
	return (uint32_t)pthread_self();
#endif
}

ThreadPool::ThreadPool() : m_threadCount(0)
{
	start();
}

void ThreadPool::queueJob(const std::function<void()>& job)
{
	printf("job is queued\n");
	{
		std::lock_guard<std::mutex> lock(m_threadData.queueMutex);
		m_threadData.jobs.push(job);
	}
	m_threadData.mutexCondition.notify_one();
}

bool ThreadPool::isBusy()
{
	std::lock_guard<std::mutex> lock(m_threadData.queueMutex);
	return !m_threadData.jobs.empty();
}

uint32_t ThreadPool::getThreadCount()
{
	return m_threadCount;
}

ThreadPool::~ThreadPool()
{
	stop();
}

#if defined(_WIN32) || defined (_WIN64)
unsigned __stdcall ThreadPool::threadLoop(void* param)
#else
	void* ThreadPool::threadLoop(void* param)
#endif
{
	auto data = static_cast<ThreadData*>(param);
	while (!data->shouldTerminate)
	{
		std::function<void()> job = nullptr;
		{
			std::unique_lock<std::mutex> lock(data->queueMutex);
			data->mutexCondition.wait(lock, [data] { return data->shouldTerminate || !data->jobs.empty(); });

			if (data->shouldTerminate)
			{
				break;
			}

			if (!data->jobs.empty())
			{
				job = data->jobs.front();
				data->jobs.pop();
			}
		}
		if (job)
		{
			printf("%u is doing it's job\n", getThreadId());
			job();
			printf("%u has done it's job\n", getThreadId());
		}
	}
	printf("%u IS TERMINATED\n", getThreadId());
#if defined(_WIN32) || defined (_WIN64)
	_endthreadex(0);
	return 0;
#else
	return nullptr;
#endif
}

void ThreadPool::start()
{
	m_threadCount = std::thread::hardware_concurrency();
	printf("THREAD COUNT = %u\n", m_threadCount);
	printf("THREAD ID | ACTION\n");
	m_threads.reserve(m_threadCount);

	std::lock_guard<std::mutex> lock(m_threadData.queueMutex);
	for (sizet i = 0; i < m_threadCount; ++i)
	{
#if defined (_WIN32) || defined (_WIN64)
		uint32_t threadId;
		uintptr_t thread = _beginthreadex(nullptr, 0, &threadLoop, &m_threadData, 0, &threadId);
		printf("%u is created \n", threadId);
		m_threads.push_back((HANDLE)(thread));
#else
		pthread_t thread;
		pthread_create(&thread,nullptr,&threadLoop,&m_threadData);
		printf("pthread is created \n");
		m_threads.push_back(thread);
#endif
	}
}

void ThreadPool::stop()
{
	{
		std::lock_guard<std::mutex> lock(m_threadData.queueMutex);
		m_threadData.shouldTerminate = true;
	}
	m_threadData.mutexCondition.notify_all();

#if defined (_WIN32) || defined (_WIN64)
	WaitForMultipleObjects(m_threadCount, m_threads.data(), TRUE, INFINITE);

	for (sizet i = 0; i < m_threadCount; ++i)
	{
		CloseHandle(m_threads[i]);
	}
#else

	for (sizet i = 0; i < m_threadCount; ++i)
	{
		pthread_join(m_threads[i],nullptr);
	}
#endif

	m_threads.clear();
	printf("THREADPOOL IS DESTROYED\n");
}

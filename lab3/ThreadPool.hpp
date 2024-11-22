#pragma once
#include <functional>
#include <mutex>
#include <queue>
#include <vector>

#if defined (_WIN32) || defined (_WIN64)
#include <Windows.h>
#endif

uint32_t getThreadId();

class ThreadPool
{
public:
	ThreadPool();
	void queueJob(const std::function<void()>& job);
	bool isBusy();
	uint32_t getThreadCount();
	~ThreadPool();

	ThreadPool(const ThreadPool& other) = delete;
	ThreadPool(ThreadPool&& other) = delete;
	ThreadPool& operator=(const ThreadPool& other) = delete;
	ThreadPool& operator=(ThreadPool&& other) = delete;

private:
	static unsigned __stdcall threadLoop(void* param);

	void start();
	void stop();

	uint32_t m_threadCount;

#if defined (_WIN32) || defined (_WIN64)
	std::vector<HANDLE> m_threads;
#endif

	struct ThreadData
	{
		std::mutex queueMutex;
		std::condition_variable mutexCondition;
		std::queue<std::function<void()>> jobs;
		bool shouldTerminate{};
	};

	ThreadData m_threadData;
};

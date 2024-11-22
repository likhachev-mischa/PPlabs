#pragma once
#include <functional>
#include <mutex>
#include <queue>
#include <vector>
#include <Windows.h>

class WindowsThreadPool
{
public:
	WindowsThreadPool();
	void queueJob(const std::function<void()>& job);
	bool isBusy();
	uint32_t getThreadCount();
	~WindowsThreadPool();

	WindowsThreadPool(const WindowsThreadPool& other) = delete;
	WindowsThreadPool(WindowsThreadPool&& other) = delete;
	WindowsThreadPool& operator=(const WindowsThreadPool& other) = delete;
	WindowsThreadPool& operator=(WindowsThreadPool&& other) = delete;

private:
	static unsigned __stdcall threadLoop(void* param);

	void start();
	void stop();

	uint32_t m_threadCount;
	std::vector<HANDLE> m_threads;

	struct ThreadData
	{
		std::mutex queueMutex;
		std::condition_variable mutexCondition;
		std::queue<std::function<void()>> jobs;
		bool shouldTerminate{};
	};

	ThreadData m_threadData;
};

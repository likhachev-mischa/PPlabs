#pragma once
#include <functional>
#include <mutex>
#include <queue>
#include <Windows.h>
#include <vector>

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
static	uint32_t threadLoop(void* param);

	void start();
	void stop();
	void joinThread(HANDLE thread);

	bool m_shouldTerminate;
	uint32_t m_threadCount;
	std::mutex m_queueMutex;
	std::condition_variable m_mutexCondition;
	std::vector<uintptr_t> m_threads;
	std::queue<std::function<void()>> m_jobs;
};

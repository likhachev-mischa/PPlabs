#pragma once
#include <functional>
#include <mutex>
#include <queue>
#include <vector>
#include <thread>
#include <condition_variable>

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
    void threadLoop();

    void start();
    void stop();
    bool m_shouldTerminate = false;
    uint32_t m_threadCount;
    std::mutex m_queueMutex;                 
    std::condition_variable m_mutexCondition;
    std::vector<std::thread> m_threads;
    std::queue<std::function<void()>> m_jobs;
};

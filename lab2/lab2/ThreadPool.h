#pragma once
#include <functional>
#include <mutex>
#include <queue>
#include <vector>

class ThreadPool
{
public:
    void start();
    void queueJob(const std::function<void()>& job);
    void stop();
    bool isBusy();
    uint32_t getThreadCount();

private:
    void threadLoop();

    bool m_shouldTerminate = false;
    uint32_t m_threadCount;
    std::mutex m_queueMutex;                 
    std::condition_variable m_mutexCondition;
    std::vector<std::thread> m_threads;
    std::queue<std::function<void()>> m_jobs;
};

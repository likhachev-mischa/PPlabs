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
    void queueJob(const std::function<void()>& job);//bind
    bool isBusy();
    uint32_t getThreadCount();
    ~ThreadPool();

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
//the rule of five, the rule of zero

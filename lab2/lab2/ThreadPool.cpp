#include "ThreadPool.h"

void ThreadPool::start()
{
    const uint32_t numThreads = std::thread::hardware_concurrency(); 
    for (uint32_t ii = 0; ii < numThreads; ++ii)
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
                return;
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

bool ThreadPool::busy()
{
    bool isPoolBusy;
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        isPoolBusy = !m_jobs.empty();
    }
    return isPoolBusy;
}

void ThreadPool::stop()
{
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_shouldTerminate = true;
    }
    m_mutexCondition.notify_all();
    for (std::thread& active_thread : m_threads)
    {
        active_thread.join();
    }
    m_threads.clear();
}
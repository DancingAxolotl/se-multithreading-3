#include "PooledThread.h"
#include <cassert>
#include <exception>

PooledThread::PooledThread(std::condition_variable& poolCv)
    : m_poolCv(poolCv)
    , m_isBusy(false)
    , m_isProcessing(true)
{
    m_thread = std::thread(&PooledThread::Process, this);
}

PooledThread::PooledThread(std::condition_variable& poolCv, std::function<void()>& task)
    : m_poolCv(poolCv)
    , m_isBusy(true)
    , m_isProcessing(true)
{
    m_task = task;
    m_thread = std::thread(&PooledThread::Process, this);
}

void PooledThread::AssignTask(std::function<void()>& task)
{
    assert(!m_isBusy);
    {
        std::lock_guard<std::mutex> lk(m_taskMutex);
        m_isBusy = true;
        m_task = task;
    }
    m_taskCv.notify_one();
}

void PooledThread::Stop()
{
    assert(!m_isBusy);
    {
        std::lock_guard<std::mutex> taskLock(m_taskMutex);
        m_isProcessing = false;
    }
    m_taskCv.notify_one();
    m_thread.join();
}

bool PooledThread::IsBusy()
{
    return m_isBusy;
}

void PooledThread::Process()
{
    std::unique_lock<std::mutex> taskLock(m_taskMutex);
    while (m_isProcessing)
    {
        if (m_isBusy)
        {
            m_task();
            m_isBusy = false;
            m_poolCv.notify_all();
        }
        m_taskCv.wait(taskLock);
    }
}

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

void PooledThread::StartTask(std::function<void()>& task)
{
    assert(!m_isBusy);
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_isBusy = true;
        m_task = task;
    }
    m_cv.notify_one();
}

void PooledThread::Stop()
{
    assert(!m_isBusy);
    {
        std::lock_guard<std::mutex> lk(m_mutex);
        m_isProcessing = false;
    }
    m_cv.notify_one();
    m_thread.join();
}

bool PooledThread::IsBusy()
{
    return m_isBusy;
}

void PooledThread::Process()
{
    while (m_isProcessing)
    {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_poolCv.notify_all();
        m_cv.wait(lk);
        if (!m_isBusy) continue;
        if (!m_isProcessing) return;
        m_task();
        m_isBusy = false;
    }
}

#include "ThreadPool.h"

PooledThread::PooledThread(std::condition_variable& poolCv)
    : m_poolCv(poolCv)
    , m_isBusy(false)
    , m_isProcessing(true)
{
    m_thread = std::thread(&PooledThread::Process, this);
}

void PooledThread::StartTask(std::function<void()>& task) {
    assert(!m_isBusy);
    std::lock_guard<std::mutex> lk(m_mutex);
    m_isBusy = true;
    m_task = task;
    m_cv.notify_one();
}

void PooledThread::Stop() {
    assert(!m_isBusy);
    std::lock_guard<std::mutex> lk(m_mutex);
    m_isProcessing = false;
    m_cv.notify_one();
}

bool PooledThread::IsBusy() {
    return m_isBusy;
}

void PooledThread::Process() {
    while (m_isProcessing) {
        std::unique_lock<std::mutex> lk(m_mutex);
        m_cv.wait(lk);
        if (!m_isBusy) continue;
        if (!m_isProcessing) return;
        m_task();
        m_isBusy = false;
        m_poolCv.notify_all();
    }
}

ThreadPool::ThreadPool(unsigned int threads)
    : threadsMax(threads)
{
}

void ThreadPool::Start()
{
    m_managerThread = std::thread(&ThreadPool::HandleTasks, this);
}

void ThreadPool::AssignTask(std::function<void()> task)
{
    {
        std::lock_guard<std::mutex> lk(m_taskMutex);
        m_tasks.push(task);
    }
    m_taskCv.notify_one();
}

PooledThread* ThreadPool::GetFreeThread()
{
    for (auto thread : m_pool) {
        if (!thread->IsBusy()) {
            return  thread;
        }
    }
    return nullptr;
}

void ThreadPool::WaitForAll()
{
    bool allFree = false;
    while (!allFree) {
        std::unique_lock<std::mutex> lk(m_poolMutex);
        m_poolCv.wait(lk);
        allFree = true;
        for (auto& thread : m_pool) {
            if (thread->IsBusy()) {
                allFree = false;
                break;
            }
        }
    }
}

void ThreadPool::HandleTasks()
{
    while(m_processing) {
        std::unique_lock<std::mutex> lk(m_taskMutex);
        if (m_tasks.empty())
        {
            m_taskCv.wait(lk);
        }

        std::function<void()> task = m_tasks.front();

        PooledThread* freeThread = GetFreeThread();
        if (freeThread != nullptr) {
            freeThread->StartTask(task);
            m_tasks.pop();
        } else {
            if (m_pool.size() < threadsMax) {
                m_pool.push_back(new PooledThread(m_poolCv));
                m_pool.back()->StartTask(task);
                m_tasks.pop();
            } else {
                lk.unlock();
                std::unique_lock<std::mutex> lk(m_poolMutex);
                m_poolCv.wait(lk);
            }
        }
    }
}

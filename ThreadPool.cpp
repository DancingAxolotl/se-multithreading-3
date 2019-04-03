#include "ThreadPool.h"
#include "PooledThread.h"
#include <cassert>
#include <exception>
#include <iostream>

ThreadPool::ThreadPool(std::size_t threads)
    : threadsMax(threads)
{
}

ThreadPool::~ThreadPool()
{
    try {
        Stop();
        for (size_t i = 0; i < m_pool.size(); ++i)
        {
            delete m_pool[i];
        }
    } catch (const std::exception) {
        std::cout << "Failed to safely stop pooled threads.";
    }
}

void ThreadPool::Start()
{
    m_managerThread = std::thread(&ThreadPool::HandleTasks, this);
}

void ThreadPool::Stop()
{
    m_processing = false;
    std::queue<std::function<void()>>().swap(m_tasks); // clear the tasks queue by swapping with an empty queue
    m_taskCv.notify_one();
    WaitForAll();
    for (auto thread : m_pool) {
        thread->Stop();
    }
    m_managerThread.join();
}

void ThreadPool::AssignTask(std::function<void()> task)
{
    {
        std::lock_guard<std::mutex> lk(m_taskMutex);
        m_tasks.push(task);
    }
    m_taskCv.notify_one();
}

void ThreadPool::WaitForAll()
{
    while (IsBusy())
    {
        std::unique_lock<std::mutex> lk(m_poolMutex);
        m_poolCv.wait(lk);
    }
}

PooledThread* ThreadPool::GetFreeThread()
{
    for (auto thread : m_pool)
    {
        if (!thread->IsBusy())
        {
            return  thread;
        }
    }
    return nullptr;
}

bool ThreadPool::IsBusy()
{
    if (!m_tasks.empty())
    {
        return true;
    }

    bool allFree = true;
    for (size_t i = 0; i < m_pool.size() && allFree; ++i)
    {
        if (m_pool[i]->IsBusy())
        {
            allFree = false;
        }
    }
    return !allFree;
}

void ThreadPool::HandleTasks()
{
    while(m_processing)
    {
        std::unique_lock<std::mutex> taskLock(m_taskMutex);
        if (!m_tasks.empty())
        {
            std::function<void()> task = m_tasks.front();
            std::unique_lock<std::mutex> poolLock(m_poolMutex);
            if (AssignToThread(task)) {
                // we managed to deal with the task
                m_tasks.pop();
            }
            else
            {
                // wait for a free thread
                taskLock.unlock();
                m_poolCv.wait(poolLock);
            }
        }
        else
        {
            // wait for a task to appear
            m_taskCv.wait(taskLock);
        }
    }
}

bool ThreadPool::AssignToThread(std::function<void()>& task)
{
    PooledThread* freeThread = GetFreeThread();
    if (freeThread != nullptr)
    {
        // send the task to a free thread
        freeThread->AssignTask(task);
        return true;
    }
    if (m_pool.size() < threadsMax)
    {
        // create a new thread to execute
        m_pool.push_back(new PooledThread(m_poolCv, task));
        return true;
    }
    return false;
}

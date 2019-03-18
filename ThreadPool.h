#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <cassert>
#include <exception>

class PooledThread {
public:
    PooledThread(std::condition_variable& poolCv);
    void StartTask(std::function<void()>& task);
    void Stop();
    bool IsBusy();

private:
    void Process();

private:
    std::mutex m_mutex;
    std::condition_variable m_cv;
    std::condition_variable& m_poolCv;
    bool m_isBusy;
    bool m_isProcessing;
    std::thread m_thread;
    std::function<void()> m_task;
};

class ThreadPool
{
public:
    ThreadPool(unsigned int threads);

    void Start();
    void AssignTask(std::function<void()> task);
    void WaitForAll();

private:
    void HandleTasks();
    PooledThread* GetFreeThread();

private:
    bool m_processing = true;
    const unsigned int threadsMax;
    std::mutex m_taskMutex;
    std::condition_variable m_taskCv;
    std::mutex m_poolMutex;
    std::condition_variable m_poolCv;
    std::thread m_managerThread;
    std::vector<PooledThread*> m_pool;
    std::queue<std::function<void()>> m_tasks;
};

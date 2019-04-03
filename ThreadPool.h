#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>

class PooledThread;

/*!
 * \brief The ThreadPool class is a pool of N threads which accepts tasks and assigns them to separate threads.
 */
class ThreadPool
{
public:
    ThreadPool(std::size_t threads);
    ~ThreadPool();

    //! Starts task processing in this thread pool.
    void Start();

    //! Stops the processing of tasks in this pool. Blocks the caller thread untill all tasks that were still processing are completed.
    void Stop();

    //! AssignTask assigns a task to this thread pool. The task will be executed as soon as a free thread is available.
    void AssignTask(std::function<void()> task);

    //! Blocks the caller thread untill all tasks from the queue are completed.
    void WaitForAll();

private:
    PooledThread* GetFreeThread();
    bool IsBusy();
    void HandleTasks();
    bool AssignToThread(std::function<void()>& task);

private:
    bool m_processing = true;
    const std::size_t threadsMax;
    std::mutex m_taskMutex;
    std::condition_variable m_taskCv;
    std::mutex m_poolMutex;
    std::condition_variable m_poolCv;
    std::thread m_managerThread;
    std::vector<PooledThread*> m_pool;
    std::queue<std::function<void()>> m_tasks;
};

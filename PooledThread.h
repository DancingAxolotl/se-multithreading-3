#pragma once
#include <thread>
#include <mutex>
#include <condition_variable>

/*!
 * \brief The PooledThread class represents a thread in a thread pool
 */
class PooledThread
{
public:
    //! Creates a free thread with no running task
    PooledThread(std::condition_variable& poolCv);
    //! Creates a busy thread which runs the specified task
    PooledThread(std::condition_variable& poolCv, std::function<void()>& task);
    //! AssignTask runs a task in a current thread if the thread is not busy
    void AssignTask(std::function<void()>& task);
    //! Stop stops processing of tasks and blocks caller untill the current task finishes
    void Stop();
    //! IsBusy returns true if there is a running task in this thread
    bool IsBusy();

private:
    void Process();
private:
    std::mutex m_taskMutex;
    std::condition_variable m_taskCv;
    std::condition_variable& m_poolCv;
    bool m_isBusy;
    bool m_isProcessing;
    std::thread m_thread;
    std::function<void()> m_task;
};

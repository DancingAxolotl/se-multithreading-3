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
    PooledThread(std::condition_variable& poolCv);
    //! StartTask runs a task in a current thread;
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

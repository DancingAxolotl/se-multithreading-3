#include <iostream>
#include <string>
#include "./ThreadPool.h"
#include "./TaskWrapper.h"

bool doThings(int taskId)
{
    std::cout << "Starting task " + std::to_string(taskId) + "\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Done " + std::to_string(taskId) + "\n";
    return taskId % 2 == 0;
}

int main()
{
    std::vector<std::future<bool>> futures;
    ThreadPool pool(10);
    pool.Start();
    for (int i = 0; i < 100; ++i)
    {
        TaskWrapper<bool, int> task(&doThings, i);
        futures.push_back(task.getFuture());
        pool.AssignTask(task);
    }
    std::cout << "Counting results...\n";
    int results = 0;
    for (auto& future : futures) {
        future.wait();
        if (future.get()) {
            results += 1;
        }
    }
    std::cout << "There were " + std::to_string(results) + " results.\n";
    return 0;
}

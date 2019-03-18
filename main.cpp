#include <iostream>
#include <thread>
#include <string>
#include "./ThreadPool.h"

void doThings(int taskId) {
    std::cout << "Starting task " + std::to_string(taskId) + "\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Done " + std::to_string(taskId) + "\n";
}

int main()
{
    ThreadPool pool(10);
    pool.Start();
    for (int i = 0; i < 100; ++i) {
        const auto fun = [i]{doThings(i);};
        pool.AssignTask(fun);
    }
    std::cout << "Waiting for tasks to finish...\n";
    pool.WaitForAll();
    return 0;
}

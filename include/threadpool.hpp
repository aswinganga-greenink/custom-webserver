#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

class ThreadPool {
   private:
    std::vector<std::thread> workers;

    std::queue<std::function<void()>> tasks;

    std::mutex              queue_mutex;
    std::condition_variable condition;

    bool stop;

   public:
    ThreadPool(size_t num_threads);
    ~ThreadPool();
    void enqueue_task(std::function<void()> task);
};
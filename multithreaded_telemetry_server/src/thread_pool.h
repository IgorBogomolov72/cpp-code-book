
#pragma once

#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <future>

class ThreadPool {
public:
    ThreadPool(size_t num_threads = std::thread::hardware_concurrency())
    {
        for (size_t i = 0; i < num_threads; ++i) {
            threads_.emplace_back([this] {
                while (true) {
                    std::packaged_task<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex_);
                        cv_.wait(lock, [this] {
                            return !tasks_.empty() || stop_;
                        });

                        if (stop_ && tasks_.empty()) {
                            return;
                        }

                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }

                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }

        cv_.notify_all();

        for (auto& thread : threads_) {
            std::cerr << "Join " << thread.get_id() << std::endl;
            thread.join();
        }
    }

    template <typename F>
    void enqueue(F&& f) {
        std::packaged_task<void()> task(std::forward<F>(f));
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            tasks_.emplace(move(task));
        }
        cv_.notify_one();
    }

private:
    std::vector<std::thread> threads_;
    std::queue<std::packaged_task<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable cv_;

    bool stop_ = false;
};
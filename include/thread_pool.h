#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <atomic>
#include <thread>
#include <vector>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <future>

// #include <stdexcept>
// #include <type_traits>
// #include <vector>

class ThreadPool
{
public:
    explicit ThreadPool(std::size_t thread_count = std::thread::hardware_concurrency())
        : stop_(false)
    {
        if (thread_count == 0) {
            thread_count = 1; // Ensure at least one thread
        }
        workers_.reserve(thread_count);
        for (std::size_t i = 0; i < thread_count; ++i) {
            workers_.emplace_back([this] {WorkerLoop();});
        } 
    }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stop_ = true;
        }
        cv_.notify_all();
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    };

    template <typename F, typename... Args>
    auto Submit(F&& f, Args&&... args) 
        -> std::future<std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>> {
        using ReturnType = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;
        
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );

        std::future<ReturnType> result = task->get_future();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stop_) {
                throw std::runtime_error("Submit on stopped ThreadPool");
            }
            tasks_.emplace([task]() { (*task)(); });
        }

        cv_.notify_one();
        return result;
    }

    std::size_t Size() const noexcept {
        return workers_.size();
    }

private:
    void WorkerLoop()
    {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(mutex_);

                cv_.wait(lock, [this] { 
                    return stop_ || !tasks_.empty();
                });

                if (stop_ && tasks_.empty()) {
                    return; // Exit if stopping and no more tasks
                }

                task = std::move(tasks_.front());
                tasks_.pop();
            }
            task();
        }
    }

    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::vector<std::thread> workers_;
    bool stop_;
};

#endif // THREAD_POOL_H
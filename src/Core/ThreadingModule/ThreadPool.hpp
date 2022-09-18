#pragma once

#include "JobQueue.hpp"      // for JobQueue
#include "WorkerThread.hpp"  // for WorkerThread

#include <algorithm>           // for generate, max
#include <condition_variable>  // for condition_variable
#include <cstdint>             // for uint8_t
#include <functional>          // for function
#include <future>              // for future, packaged_task
#include <memory>              // for make_shared
#include <mutex>               // for mutex
#include <thread>              // for thread
#include <vector>              // for vector

#define MAX_THREADS std::max((int) std::thread::hardware_concurrency() - 1, 1)

class ThreadPool {
public:
    template<typename F, typename... Args>
    auto submit(F &f, Args &&...args) -> std::future<decltype(f(args...))> {
        auto task_ptr =
          std::make_shared<std::packaged_task<decltype(f(args...))()>>([f, args...] { return f(args...); });
        std::function<void()> wrapper_func = [task_ptr]() {
            (*task_ptr)();
        };
        m_queue.enqueue(wrapper_func);
        m_condition.notify_one();
        return task_ptr->get_future();
    }

    explicit ThreadPool(uint8_t threadCount = MAX_THREADS) {
        std::generate(
          m_threads.begin(),
          m_threads.begin() + threadCount,
          [&]() -> std::thread { return std::thread(WorkerThread(this)); }
        );
    }

    std::condition_variable         m_condition;
    JobQueue<std::function<void()>> m_queue{};
    std::vector<std::thread>        m_threads;
    std::condition_variable         m_conditional_lock;
    std::mutex                      m_conditional_mutex;
    bool                            m_shutdown{};

    ThreadPool(const ThreadPool &) = delete;

    ThreadPool(ThreadPool &&) = delete;

    ThreadPool &operator=(const ThreadPool &) = delete;

    ThreadPool &operator=(ThreadPool &&) = delete;
};
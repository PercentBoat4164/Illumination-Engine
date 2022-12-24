#pragma once

#include "JobQueue.hpp"  // for JobQueue

#include <algorithm>           // for max
#include <condition_variable>  // for condition_variable
#include <cstdint>             // for uint8_t
#include <functional>          // for function, bind
#include <future>              // for future, packaged_task
#include <memory>              // for make_shared
#include <mutex>               // for mutex
#include <thread>              // for thread
#include <utility>             // for forward
#include <vector>              // for vector

#define MAX_THREADS std::max(static_cast<uint8_t>(std::thread::hardware_concurrency() - 1U), uint8_t{1U})

namespace IE::Core {
class ThreadPool {
public:
    template<typename F, typename... Args>
    auto submit(F &&f, Args &&...args) -> std::future<decltype(f(args...))> {
        auto                  task_ptr{std::make_shared<std::packaged_task<decltype(f(args...))()>>(
          [f, ... args = std::forward<Args>(args)] { return f(args...); }
        )};
        std::function<void()> wrapper_func{[task_ptr]() {
            (*task_ptr)();
        }};
        m_queue.enqueue(wrapper_func);
        m_conditional_lock.notify_one();
        return task_ptr->get_future();
    }

    explicit ThreadPool(uint8_t threadCount = MAX_THREADS);

    ThreadPool(const ThreadPool &) = delete;

    ThreadPool(ThreadPool &&) = delete;

    ThreadPool &operator=(const ThreadPool &) = delete;

    ThreadPool &operator=(ThreadPool &&) = delete;

    ~ThreadPool();

public:
    [[nodiscard]] std::condition_variable &getCondition();

    [[nodiscard]] IE::Core::detail::JobQueue<std::function<void()>> &getQueue();

    [[nodiscard]] std::vector<std::thread> &getThreads();

    [[nodiscard]] std::condition_variable &getConditionalLock();

    [[nodiscard]] std::mutex &getConditionalMutex();

    [[nodiscard]] bool isShutdown() const;

    void shutdown();

private:
    std::condition_variable                           m_condition;
    IE::Core::detail::JobQueue<std::function<void()>> m_queue;
    std::vector<std::thread>                          m_threads;
    std::condition_variable                           m_conditional_lock;
    std::mutex                                        m_conditional_mutex;
    bool                                              m_shutdown;
};
}  // namespace IE::Core
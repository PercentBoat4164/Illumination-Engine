#pragma once

#include "CoroutineTask.hpp"
#include "EnsureThread.hpp"
#include "FunctionTask.hpp"
#include "Queue.hpp"
#include "Task.hpp"
#include "Worker.hpp"

#if defined(AppleClang)
#    include <experimental/coroutine>
#else
#    include <coroutine>
#endif
#include <atomic>
#include <condition_variable>
#include <functional>
#include <thread>

/**@todo Enable waiting on non-thread pool related things. (e.g. sleep(1))*/
namespace IE::Core::Threading {
class ThreadPool {
    std::vector<std::thread>         m_workers;
    Queue<std::shared_ptr<BaseTask>> m_queue;
    Queue<std::shared_ptr<BaseTask>> m_mainQueue;
    std::condition_variable_any      m_workAssignmentConditionVariable;
    std::atomic<bool>                m_shutdown{false};

public:
    explicit ThreadPool(size_t threads = std::thread::hardware_concurrency());

    void startMainThreadLoop() {
        ThreadPool &pool = *this;
        std::shared_ptr<BaseTask> task;
        std::mutex mutex;
        while (!pool.m_shutdown) {
            std::unique_lock<std::mutex> lock(mutex);
            if (!pool.m_mainQueue.pop(task))
                pool.m_workAssignmentConditionVariable.wait(lock, [&] -> bool {
                    return pool.m_mainQueue.pop(task) || pool.m_shutdown;
                });
            if (pool.m_shutdown) break;
            task->execute();
        }
    }

    template<typename T>
    auto submit(CoroutineTask<T> &&t_coroutine) -> std::shared_ptr<Task<T>> {
        auto task{std::make_shared<CoroutineTask<T>>(t_coroutine)};
        task->connectHandle();
            m_queue.push(std::static_pointer_cast<BaseTask>(task));
        m_workAssignmentConditionVariable.notify_one();
        return std::static_pointer_cast<Task<T>>(task);
    }

    template<typename T, typename... Args>
        requires requires(T &&t_coroutine, Args &&...args) { typename decltype(t_coroutine(args...))::ReturnType; }
    auto submit(T &&t_coroutine, Args &&...args)
      -> std::shared_ptr<Task<typename decltype(t_coroutine(args...))::ReturnType>> {
        auto task{
          std::make_shared<CoroutineTask<typename decltype(t_coroutine(args...))::ReturnType>>(t_coroutine(args...)
          )};
        task->connectHandle();
        m_queue.push(std::static_pointer_cast<BaseTask>(task));
        m_workAssignmentConditionVariable.notify_one();
        return task;
    }

    template<typename T, typename... Args>
    auto submit(T &&t_function, Args &&...args) -> std::shared_ptr<Task<decltype(t_function(args...))>> {
        auto job{std::make_shared<FunctionTask<decltype(t_function(args...))>>(
          [t_function, ... args = std::forward<Args>(args)] { return t_function(args...); }
        )};
        m_queue.push(std::static_pointer_cast<BaseTask>(job));
        m_workAssignmentConditionVariable.notify_one();
        return job;
    }

    template<typename... Args>
    ResumeAfter resumeAfter(Args... args) {
        return ResumeAfter{this, args...};
    }

    ResumeAfter resumeAfter(const std::vector<std::shared_ptr<BaseTask>> &t_tasks);

    EnsureThread ensureThread(ThreadType t_type) {
        return EnsureThread(this, t_type);
    }

    ~ThreadPool();

    uint32_t getWorkerCount();

    friend void Worker::start(ThreadPool *t_threadPool);
};
}  // namespace IE::Core::Threading
#pragma once

#include "CoroutineTask.hpp"
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
    Queue<std::shared_ptr<BaseTask>> m_activeQueue;
    std::condition_variable_any      m_workAssignmentConditionVariable;
    std::atomic<bool>                m_shutdown{false};

public:
    explicit ThreadPool(size_t threads = std::thread::hardware_concurrency());

    template<typename T>
    auto submit(CoroutineTask<T> &&t_coroutine) -> std::shared_ptr<Task<T>> {
        auto task{std::make_shared<CoroutineTask<T>>(t_coroutine)};
        task->connectHandle();
        m_activeQueue.push(std::static_pointer_cast<BaseTask>(task));
        m_workAssignmentConditionVariable.notify_one();
        return std::static_pointer_cast<Task<T>>(task);
    }

    template<typename T, typename... Args>
        requires requires(T &&t_coroutine, Args &&...args) { typename decltype(t_coroutine(args...))::ReturnType; }
    auto submit(T &&t_coroutine, Args &&...args) -> std::shared_ptr<Task<typename decltype(t_coroutine(args...))::ReturnType>> {
        auto task{std::make_shared<CoroutineTask<typename decltype(t_coroutine(args...))::ReturnType>>(t_coroutine(args...))};
        task->connectHandle();
        m_activeQueue.push(std::static_pointer_cast<BaseTask>(task));
        m_workAssignmentConditionVariable.notify_one();
        return task;
    }

    template<typename T, typename... Args>
    auto submit(T &&t_function, Args &&...args) -> std::shared_ptr<Task<decltype(t_function(args...))>> {
        auto job{std::make_shared<FunctionTask<decltype(t_function(args...))>>([t_function, ... args = std::forward<Args>(args)] {
            return t_function(args...);
        })};
        m_activeQueue.push(std::static_pointer_cast<BaseTask>(job));
        m_workAssignmentConditionVariable.notify_one();
        return job;
    }

    template<typename... Args>
    ResumeAfter resumeAfter(Args... args) {
        return ResumeAfter{this, args...};
    }

    ResumeAfter resumeAfter(const std::vector<std::shared_ptr<BaseTask>> &t_tasks);

    ~ThreadPool();

    uint32_t getWorkerCount();

    friend void Worker::start(ThreadPool *t_threadPool);
#if defined(AppleClang)
    friend void ResumeAfter::await_suspend(std::experimental::coroutine_handle<> t_handle);
#else
    friend void ResumeAfter::await_suspend(std::coroutine_handle<> t_handle);
#endif
};
}  // namespace IE::Core::Threading
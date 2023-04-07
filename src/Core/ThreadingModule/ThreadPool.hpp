#pragma once

#include "Core/ThreadingModule/ResumeAfter.hpp"
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
    std::condition_variable_any      m_workAssignedNotifier;
    std::condition_variable_any      m_mainWorkAssignedNotifier;
    std::atomic<bool>                m_shutdown{false};
    std::thread::id                  mainThreadID;

public:
    explicit ThreadPool(size_t threads = std::thread::hardware_concurrency());

    void startMainThreadLoop();

    template<typename T>
    auto submit(CoroutineTask<T> &&t_coroutine) -> std::shared_ptr<Task<T>> {
        auto task{std::make_shared<CoroutineTask<T>>(t_coroutine)};
        task->connectHandle();
        m_queue.push(std::static_pointer_cast<BaseTask>(task));
        m_workAssignedNotifier.notify_one();
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
        m_workAssignedNotifier.notify_one();
        return task;
    }

    template<typename T, typename... Args>
    auto submit(T &&t_function, Args &&...args) -> std::shared_ptr<Task<decltype(t_function(args...))>> {
        auto task{std::make_shared<FunctionTask<decltype(t_function(args...))>>(
          [t_function, ... args = std::forward<Args>(args)] { return t_function(args...); }
        )};
        m_queue.push(std::static_pointer_cast<BaseTask>(task));
        m_workAssignedNotifier.notify_one();
        return task;
    }

    template<typename T>
    auto submitToMainThread(CoroutineTask<T> &&t_coroutine) -> std::shared_ptr<Task<T>> {
        auto task{std::make_shared<CoroutineTask<T>>(t_coroutine)};
        task->connectHandle();
        m_mainQueue.push(std::static_pointer_cast<BaseTask>(task));
        m_mainWorkAssignedNotifier.notify_one();
        return std::static_pointer_cast<Task<T>>(task);
    }

    template<typename T, typename... Args>
        requires requires(T &&t_coroutine, Args &&...args) { typename decltype(t_coroutine(args...))::ReturnType; }
    auto submitToMainThread(T &&t_coroutine, Args &&...args)
      -> std::shared_ptr<Task<typename decltype(t_coroutine(args...))::ReturnType>> {
        auto task{
          std::make_shared<CoroutineTask<typename decltype(t_coroutine(args...))::ReturnType>>(t_coroutine(args...)
          )};
        task->connectHandle();
        m_mainQueue.push(std::static_pointer_cast<BaseTask>(task));
        m_mainWorkAssignedNotifier.notify_one();
        return task;
    }

    template<typename T, typename... Args>
    auto submitToMainThread(T &&t_function, Args &&...args)
      -> std::shared_ptr<Task<decltype(t_function(args...))>> {
        auto task{std::make_shared<FunctionTask<decltype(t_function(args...))>>(
          [t_function, ... args = std::forward<Args>(args)] { return t_function(args...); }
        )};
        m_mainQueue.push(std::static_pointer_cast<BaseTask>(task));
        m_mainWorkAssignedNotifier.notify_one();
        return task;
    }

    template<typename... Args>
    ResumeAfter resumeAfter(Args... args) {
        return ResumeAfter{this, args...};
    }

    EnsureThread ensureThread(ThreadType t_type) {
        return {this, t_type};
    }

    ~ThreadPool();

    uint32_t getWorkerCount();

    friend void Worker::start(ThreadPool *t_threadPool);
    friend bool EnsureThread::await_ready();
};
}  // namespace IE::Core::Threading
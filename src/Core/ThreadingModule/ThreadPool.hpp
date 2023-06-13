#pragma once

#include "Core/ThreadingModule/Awaitable.hpp"
#include "EnsureThread.hpp"
#include "Queue.hpp"
#include "ResumeAfter.hpp"
#include "Task.hpp"
#include "Worker.hpp"

#include <memory>

#if defined(AppleClang)
#    include <experimental/coroutine>

namespace std {
using std::experimental::coroutine_handle;
using std::experimental::suspend_always;
using std::experimental::suspend_never;
}  // namespace std
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
    std::atomic<bool>                m_mainShutdown{false};
    std::thread::id                  mainThreadID;
    std::atomic<uint32_t>            m_threadShutdownCount{0};

    template<typename T>
    std::shared_ptr<Task<T>>
    prepareAndSubmit(std::shared_ptr<Task<T>> t_task, ThreadType t_threadType = IE_THREAD_TYPE_WORKER_THREAD) {
        t_task->connectHandle();
        bool worker = t_threadType == IE_THREAD_TYPE_WORKER_THREAD;
        (worker ? m_queue : m_mainQueue).push(std::static_pointer_cast<BaseTask>(t_task));
        (worker ? m_workAssignedNotifier : m_mainWorkAssignedNotifier).notify_one();
        return t_task;
    }

public:
    explicit ThreadPool(uint32_t t_threads = std::thread::hardware_concurrency());

    void startMainThreadLoop();

    template<typename T>
    std::shared_ptr<Task<T>> submit(Task<T> t_coroutine) {
        return submit(thisThreadType(), t_coroutine);
    }

    template<typename T>
    std::shared_ptr<Task<T>> submit(ThreadType t_threadType, Task<T> t_coroutine) {
        return prepareAndSubmit(std::make_shared<Task<T>>(t_coroutine), t_threadType);
    }

    template<typename T, typename... Args>
    auto submit(T &&t_coroutine, Args &&...args)
      -> std::shared_ptr<Task<typename decltype(t_coroutine(args...))::ReturnType>> {
        return submit(thisThreadType(), t_coroutine, args...);
    }

    template<typename T, typename... Args>
        requires requires(T &&t_coroutine, Args &&...args) { typename decltype(t_coroutine(args...))::ReturnType; }
    auto submit(ThreadType t_threadType, T &&t_coroutine, Args &&...args)
      -> std::shared_ptr<Task<typename decltype(t_coroutine(args...))::ReturnType>> {
        return prepareAndSubmit(
          std::make_shared<Task<typename decltype(t_coroutine(args...))::ReturnType>>(t_coroutine(args...)),
          t_threadType
        );
    }

    template<typename T, typename... Args>
    auto submit(T &&t_function, Args &&...args) -> std::shared_ptr<Task<decltype(t_function(args...))>> {
        return submit(thisThreadType(), t_function, args...);
    }

    template<typename T, typename... Args>
    auto submit(ThreadType t_threadType, T &&t_function, Args &&...args)
      -> std::shared_ptr<Task<decltype(t_function(args...))>> {
        return prepareAndSubmit(
          std::make_shared<Task<decltype(t_function(args...))>>(
            [](T &&function, Args &&...args) -> Task<decltype(t_function(args...))> {
                co_return function(args...);
            }(t_function, args...)
          ),
          t_threadType
        );
    }

    std::shared_ptr<Task<void>> submit(std::coroutine_handle<> t_handle);

    std::shared_ptr<Task<void>> submit(ThreadType t_threadType, std::coroutine_handle<> t_handle);

    template<typename T>
    T executeInPlace(Task<T> t_coroutine) {
        auto coroutine{submit(t_coroutine)};
        Worker::waitForTask(this, *std::static_pointer_cast<BaseTask>(coroutine));
        return coroutine->value();
    }

    template<typename T, typename... Args>
        requires requires(T &&t_coroutine, Args &&...args) { typename decltype(t_coroutine(args...))::ReturnType; }
    auto executeInPlace(T &&t_coroutine, Args &&...args) -> typename decltype(t_coroutine(args...))::ReturnType {
        auto coroutine{submit(t_coroutine, args...)};
        Worker::waitForTask(this, *std::static_pointer_cast<BaseTask>(coroutine));
        if constexpr (std::is_void_v<typename decltype(t_coroutine(args...))::ReturnType>) return;
        return coroutine->value();
    }

    void awakenAll();

    template<typename... Args>
    ResumeAfter resumeAfter(ThreadType t_threadType, Args... args) {
        return ResumeAfter{this, t_threadType, args...};
    }

    template<typename... Args>
    ResumeAfter resumeAfter(Args... args) {
        return ResumeAfter{this, thisThreadType(), args...};
    }

    EnsureThread ensureThread(ThreadType t_type);

    ThreadType thisThreadType() {
        return std::this_thread::get_id() == mainThreadID ? IE_THREAD_TYPE_MAIN_THREAD :
                                                            IE_THREAD_TYPE_WORKER_THREAD;
    }

    ~ThreadPool();

    uint32_t getWorkerCount();

    void shutdown();

    void setWorkerCount(uint32_t t_threads = std::thread::hardware_concurrency());

    friend void Worker::start(ThreadPool *t_threadPool);
    friend void Worker::waitForTask(ThreadPool *t_threadPool, BaseTask &t_task);

    friend bool EnsureThread::await_ready();
};
}  // namespace IE::Core::Threading
#pragma once

#include "CoroutineTask.hpp"
#include "FunctionTask.hpp"
#include "Pool.hpp"
#include "Queue.hpp"
#include "Task.hpp"

#if defined(AppleClang)
#    include <experimental/coroutine>
#else
#    include <coroutine>
#endif
#include <atomic>
#include <condition_variable>
#include <functional>
#include <thread>

namespace IE::Core::Threading {
class ThreadPool;

struct ResumeAfter {
    ResumeAfter() = default;

    template<typename... Args>
    explicit ResumeAfter(ThreadPool *t_threadPool, Args... args) :
            m_ready([args...] { return (... && args->finished()); }),
            m_threadPool(t_threadPool) {
    }

    ResumeAfter(ThreadPool *t_threadPool, const std::vector<std::shared_ptr<BaseTask>> &t_tasks) :
            m_ready([t_tasks] {
                return std::all_of(t_tasks.begin(), t_tasks.end(), [](const std::shared_ptr<BaseTask> &task) {
                    return task->finished();
                });
            }),
            m_threadPool(t_threadPool) {
    }

    // Indicates the readiness of the coroutine to continue. True -> resume, False -> suspend
    bool await_ready() {
        return m_ready();
    }

#if defined(AppleClang)
    void await_suspend(std::experimental::coroutine_handle<> t_handle);
#else
    void                    await_suspend(std::coroutine_handle<> t_handle);
#endif

    void await_resume() {
    }

    void resume() {
        m_handle.resume();
    }

private:
    ThreadPool           *m_threadPool{};
    std::function<bool()> m_ready;
#if defined(AppleClang)
    std::experimental::coroutine_handle<> m_handle;
#else
    std::coroutine_handle<> m_handle;
#endif
} __attribute__((aligned(64)));

class Worker {
public:
    Worker() = default;

    void start(ThreadPool *t_threadPool);
};

class ThreadPool {
    std::vector<std::thread>         m_workers;
    Queue<std::shared_ptr<BaseTask>> m_activeQueue;
    Pool<ResumeAfter>                m_suspendedPool;
    std::condition_variable_any      m_workAssignmentConditionVariable;
    std::atomic<bool>                m_shutdown{false};

public:
    explicit ThreadPool(size_t threads = std::thread::hardware_concurrency()) {
        m_workers.reserve(threads);
        for (; threads > 0; --threads) m_workers.emplace_back([this] { Worker().start(this); });
    }

    template<typename T>
    auto submit(CoroutineTask<T> &&f) -> std::shared_ptr<Task<T>> {
        auto job{std::make_shared<CoroutineTask<T>>(f)};
        job->connectHandle();
        m_activeQueue.push(std::static_pointer_cast<BaseTask>(job));
        m_workAssignmentConditionVariable.notify_one();
        return std::static_pointer_cast<Task<T>>(job);
    }

    template<typename T, typename... Args>

        requires requires(T &&f, Args &&...args) { typename decltype(f(args...))::ReturnType; }

    auto submit(T &&f, Args &&...args) -> std::shared_ptr<Task<typename decltype(f(args...))::ReturnType>> {
        auto job{std::make_shared<CoroutineTask<typename decltype(f(args...))::ReturnType>>(f(args...))};
        job->connectHandle();
        m_activeQueue.push(std::static_pointer_cast<BaseTask>(job));
        m_workAssignmentConditionVariable.notify_one();
        return job;
    }

    template<typename T, typename... Args>
    auto submit(T &&f, Args &&...args) -> std::shared_ptr<Task<decltype(f(args...))>> {
        auto job{std::make_shared<FunctionTask<decltype(f(args...))>>([f, ... args = std::forward<Args>(args)] {
            return f(args...);
        })};
        m_activeQueue.push(std::static_pointer_cast<BaseTask>(job));
        m_workAssignmentConditionVariable.notify_one();
        return job;
    }

    template<typename... Args>
    ResumeAfter resumeAfter(Args... args) {
        std::this_thread::yield();
        return ResumeAfter{this, args...};
    }

    ResumeAfter resumeAfter(const std::vector<std::shared_ptr<BaseTask>> &t_tasks) {
        std::this_thread::yield();
        return ResumeAfter{this, t_tasks};
    }

    ~ThreadPool() {
        m_shutdown = true;
        m_workAssignmentConditionVariable.notify_all();
        for (std::thread &thread : m_workers)
            if (thread.joinable()) thread.join();
    }

    uint32_t getWorkerCount() {
        return m_workers.size();
    }

    friend void Worker::start(ThreadPool *t_threadPool);
#if defined(AppleClang)
    friend void ResumeAfter::await_suspend(std::experimental::coroutine_handle<> t_handle);
#else
    friend void             ResumeAfter::await_suspend(std::coroutine_handle<> t_handle);
#endif
};
}  // namespace IE::Core::Threading
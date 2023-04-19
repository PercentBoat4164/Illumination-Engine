#pragma once

#include "ResumeAfter.hpp"
#include "Task.hpp"

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
#include <functional>
#include <mutex>

namespace IE::Core::Threading {
template<typename T>
class CoroutineTask : public Task<T> {
public:
    using ReturnType = T;

    struct promise_type {
        CoroutineTask<ReturnType> *parent;

        CoroutineTask<ReturnType> get_return_object() {
            return CoroutineTask<ReturnType>{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() noexcept {
            return {};
        }

        std::suspend_never final_suspend() noexcept {
            CoroutineTask<ReturnType> &p = *parent;
            {
                std::lock_guard<std::mutex> lock(*p.m_dependentsMutex);
                for (Awaitable *dependent : p.m_dependents)
                    static_cast<ResumeAfter *>(dependent)->releaseDependency();
            }
            p.m_dependents.clear();
            *p.m_finished = true;
            p.m_finishedNotifier->notify_all();
            return {};
        }

        void unhandled_exception() {
            std::rethrow_exception(std::current_exception());
        }

        void return_value(T t_value) {
            parent->m_value = t_value;
        }

        explicit operator T() {
            return parent->m_value;
        }
    };

    virtual ~CoroutineTask() = default;

    explicit CoroutineTask(std::coroutine_handle<promise_type> t_handle) : m_handle(t_handle) {
    }

    explicit operator std::coroutine_handle<promise_type>() {
        return m_handle;
    }

    void execute() override {
        m_handle.resume();
    }

    void connectHandle() {
        m_handle.promise().parent = this;
    }

    void wait() override {
        std::mutex                   mutex;
        std::unique_lock<std::mutex> lock(mutex);
        if (!*BaseTask::m_finished)
            BaseTask::m_finishedNotifier->wait(lock, [&] -> bool { return *BaseTask::m_finished; });
    }

private:
    std::coroutine_handle<promise_type> m_handle;
};

template<>
class CoroutineTask<void> : public Task<void> {
public:
    using ReturnType = void;

    struct promise_type {
        CoroutineTask<ReturnType> *parent;

        CoroutineTask<ReturnType> get_return_object() {
            return CoroutineTask<ReturnType>{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() noexcept {
            return {};
        }

        std::suspend_never final_suspend() noexcept {
            CoroutineTask<void> &p = *parent;
            {
                std::lock_guard<std::mutex> lock(*p.m_dependentsMutex);
                for (Awaitable *dependent : p.m_dependents)
                    static_cast<ResumeAfter *>(dependent)->releaseDependency();
            }
            p.m_dependents.clear();
            *p.m_finished = true;
            p.m_finishedNotifier->notify_all();
            return {};
        }

        void unhandled_exception() {
            std::rethrow_exception(std::current_exception());
        }

        void return_void() {}
    };

    virtual ~CoroutineTask() = default;

    explicit CoroutineTask(std::coroutine_handle<promise_type> t_handle) : m_handle(t_handle) {
    }

    explicit operator std::coroutine_handle<promise_type>() {
        return m_handle;
    }

    void execute() override {
        m_handle.resume();
    }

    void connectHandle() {
        m_handle.promise().parent = this;
    }

    void wait() override {
        std::mutex                   mutex;
        std::unique_lock<std::mutex> lock(mutex);
        if (!*BaseTask::m_finished)
            BaseTask::m_finishedNotifier->wait(lock, [&] -> bool { return *BaseTask::m_finished; });
    }

private:
    std::coroutine_handle<promise_type> m_handle;
};
}  // namespace IE::Core::Threading
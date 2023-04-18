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
        CoroutineTask<T> *parent;

        CoroutineTask<T> get_return_object() {
            return CoroutineTask<T>{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() noexcept {
            return {};
        }

        std::suspend_never final_suspend() noexcept {
            {
                std::lock_guard<std::mutex> lock(*parent->m_dependentsMutex);
                for (Awaitable *dependent : parent->m_dependents)
                    static_cast<ResumeAfter *>(dependent)->releaseDependency();
            }
            parent->m_dependents.clear();
            *parent->m_finished = true;
            parent->m_finishedNotifier->notify_all();
            return {};
        }

        void unhandled_exception() {
            std::rethrow_exception(std::current_exception());
        }

        std::suspend_always yield_value(T t_value) {
            parent->m_value = t_value;
            return {};
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
        if (!*(BaseTask::m_finished))
            BaseTask::m_finishedNotifier->wait(lock, [&] { return BaseTask::m_finished->operator bool(); });
    }

private:
    std::coroutine_handle<promise_type> m_handle;
};  // namespace IE::Core::Threading

template<>
class CoroutineTask<void> : public Task<void> {
public:
    using ReturnType = void;

    struct promise_type {
        CoroutineTask<void> *parent;

        CoroutineTask<void> get_return_object() {
            return CoroutineTask<void>{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() noexcept {
            return {};
        }

        std::suspend_never final_suspend() noexcept {
            {
                std::lock_guard<std::mutex> lock(*parent->m_dependentsMutex);
                for (Awaitable *dependent : parent->m_dependents)
                    static_cast<ResumeAfter *>(dependent)->releaseDependency();
            }
            parent->m_dependents.clear();
            *parent->m_finished = true;
            parent->m_finishedNotifier->notify_all();
            return {};
        }

        void unhandled_exception() {
            std::rethrow_exception(std::current_exception());
        }

        std::suspend_always yield_value() {
            return {};
        }

        void return_void() {
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
        if (!*m_finished) {
            std::mutex                   mutex;
            std::unique_lock<std::mutex> lock(mutex);
            m_finishedNotifier->wait(lock, [&] { return m_finished->operator bool(); });
        }
    }

private:
    std::coroutine_handle<promise_type> m_handle;
};
}  // namespace IE::Core::Threading
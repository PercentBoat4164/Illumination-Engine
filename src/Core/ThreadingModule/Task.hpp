#pragma once

#include "Awaitable.hpp"
#include "BaseTask.hpp"

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
class Task : public BaseTask {
public:
    using ReturnType = T;

    struct promise_type {
        Task<T> *parent;

        Task<T> get_return_object() {
            return Task<T>{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() noexcept {
            return {};
        }

        std::suspend_never final_suspend() noexcept {
            {
                std::lock_guard<std::mutex> lock(*parent->m_dependentsMutex);
                for (Awaitable *dependent : parent->m_dependents) dependent->releaseDependency();
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

    Task(std::coroutine_handle<promise_type> t_handle) : m_handle(t_handle) {
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

    T value() {
        return m_value;
    }

    operator T() {
        return m_value;
    }

private:
    std::coroutine_handle<promise_type> m_handle;
    T                                   m_value;
};  // namespace IE::Core::Threading

template<>
class Task<void> : public BaseTask {
public:
    using ReturnType = void;

    struct promise_type {
        Task<void> *parent;

        Task<void> get_return_object() {
            return Task<void>{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() noexcept {
            return {};
        }

        std::suspend_never final_suspend() noexcept {
            {
                std::lock_guard<std::mutex> lock(*parent->m_dependentsMutex);
                for (Awaitable *dependent : parent->m_dependents) dependent->releaseDependency();
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

    Task(std::coroutine_handle<promise_type> t_handle) : m_handle(t_handle) {
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

    void value(){};

private:
    std::coroutine_handle<promise_type> m_handle;
};
}  // namespace IE::Core::Threading
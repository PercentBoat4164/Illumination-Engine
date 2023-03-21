#pragma once

#include "Task.hpp"

#if defined(AppleClang)
#    include <experimental/coroutine>
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
#if defined(AppleClang)
            return CoroutineTask<T>{std::experimental::coroutine_handle<promise_type>::from_promise(*this)};
#else
            return CoroutineTask<T>{std::coroutine_handle<promise_type>::from_promise(*this)};
#endif
        }

#if defined(AppleClang)
        std::experimental::suspend_always initial_suspend() noexcept {
#else
        std::suspend_always initial_suspend() noexcept {
#endif
            return {};
        }

#if defined(AppleClang)
        std::experimental::suspend_never final_suspend() noexcept {
#else
        std::suspend_never  final_suspend() noexcept {
#endif
            *parent->m_finished = true;
            parent->m_finishedNotifier->notify_all();
            return {};
        }

        void unhandled_exception() {
            std::rethrow_exception(std::current_exception());
        }

#if defined(AppleClang)
        std::experimental::suspend_always yield_value(T t_value){
#else
        std::suspend_always yield_value(T t_value) {
#endif
          parent->m_value = t_value;
        return {};

    }

    void
    return_value(T t_value) {

        parent->m_value = t_value;
    }

    operator T() {
        return parent->m_value;
    }
};

#if defined(AppleClang)
explicit CoroutineTask(std::experimental::coroutine_handle<promise_type> t_handle) : m_handle(t_handle) {
#else
    explicit CoroutineTask(std::coroutine_handle<promise_type> t_handle) : m_handle(t_handle) {
#endif
}

#if defined(AppleClang)
operator const std::experimental::coroutine_handle<promise_type>() {
#else
                                        operator const std::coroutine_handle<promise_type>() {
#endif
    return m_handle;
}

void execute() override {
    m_handle.resume();
}

void connectHandle() {
    m_handle.promise().parent = this;
}

void wait() override {
    if (!*(BaseTask::m_finished)) {
        std::mutex                   mutex;
        std::unique_lock<std::mutex> lock(mutex);
        BaseTask::m_finishedNotifier->wait(lock, [&] { return BaseTask::m_finished->operator bool(); });
    }
}

private:
#if defined(AppleClang)
std::experimental::coroutine_handle<promise_type> m_handle;
#else
    std::coroutine_handle<promise_type> m_handle;
#endif
};  // namespace IE::Core::Threading

template<>
class CoroutineTask<void> : public Task<void> {
public:
    using ReturnType = void;

    struct promise_type {
        CoroutineTask<void> *parent;

        CoroutineTask<void> get_return_object() {
#if defined(AppleClang)
            return CoroutineTask<void>{std::experimental::coroutine_handle<promise_type>::from_promise(*this)};
#else
            return CoroutineTask<void>{std::coroutine_handle<promise_type>::from_promise(*this)};
#endif
        }

#if defined(AppleClang)
        std::experimental::suspend_always initial_suspend() noexcept {
#else
        std::suspend_always initial_suspend() noexcept {
#endif
            return {};
        }

#if defined(AppleClang)
        std::experimental::suspend_never final_suspend() noexcept {
#else
        std::suspend_never  final_suspend() noexcept {
#endif
            *parent->m_finished = true;
            parent->m_finishedNotifier->notify_all();
            return {};
        }

        void unhandled_exception() {
            std::rethrow_exception(std::current_exception());
        }

#if defined(AppleClang)
        std::experimental::suspend_always yield_value(){
#else
        std::suspend_always yield_value() {
#endif
          return {};
    }
};

#if defined(AppleClang)
CoroutineTask(std::experimental::coroutine_handle<promise_type> t_handle) : m_handle(t_handle) {
#else
    CoroutineTask(std::coroutine_handle<promise_type> t_handle) : m_handle(t_handle) {
#endif
}

#if defined(AppleClang)
operator const std::experimental::coroutine_handle<promise_type>() {
#else
                                        operator const std::coroutine_handle<promise_type>() {
#endif
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
#if defined(AppleClang)
std::experimental::coroutine_handle<promise_type> m_handle;
#else
    std::coroutine_handle<promise_type> m_handle;
#endif
}
;
}  // namespace IE::Core::Threading
#pragma once

#include "Task.hpp"

#include <coroutine>
#include <functional>
#include <mutex>

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

        operator T() {
            return parent->m_value;
        }
    };

    CoroutineTask(std::coroutine_handle<promise_type> t_handle) : m_handle(t_handle) {
    }

    operator const std::coroutine_handle<promise_type>() {
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
            BaseTask::m_finishedNotifier->wait(lock);
        }
    }

private:
    std::coroutine_handle<promise_type> m_handle;
};

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
    };

    CoroutineTask(std::coroutine_handle<promise_type> t_handle) : m_handle(t_handle) {
    }

    operator const std::coroutine_handle<promise_type>() {
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
            BaseTask::m_finishedNotifier->wait(lock);
        }
    }

private:
    std::coroutine_handle<promise_type> m_handle;
};
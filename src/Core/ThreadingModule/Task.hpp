#pragma once

#include "Awaitable.hpp"
#include "BaseTask.hpp"

#include <type_traits>

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
class Task;

namespace detail {
template<typename T, bool B>
struct ConditionalInheritance : T {};

template<typename T>
struct ConditionalInheritance<T, false> {};

template<typename T0, typename T1, bool B>
struct Choose : ConditionalInheritance<T0, B>, ConditionalInheritance<T1, not B> {};

template<typename T>
struct Member {
    T m_value;
};

template<typename T>
struct promise_type {
    Task<T> *parent;

    Task<T> get_return_object();

    std::suspend_always initial_suspend() noexcept {
        return {};
    }

    std::suspend_never final_suspend() noexcept;

    void unhandled_exception() {
        std::rethrow_exception(std::current_exception());
    }

    operator T();
};

template<typename T, bool B>
struct return_promise_type : promise_type<T> {
    void return_value(T t_value);
};

template<typename T>
struct return_promise_type<T, false> : promise_type<T> {
    void return_void() {
    }
};
}  // namespace detail

template<typename T>
class Task : detail::ConditionalInheritance<detail::Member<T>, not std::is_void_v<T>>, public BaseTask {
public:
    using ReturnType = T;

    using promise_type = detail::return_promise_type<ReturnType, not std::is_void_v<ReturnType>>;

    explicit Task(std::coroutine_handle<promise_type> t_handle) : m_handle(t_handle) {
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

    ReturnType value() {
        if constexpr (not std::is_void_v<ReturnType>)
            return detail::ConditionalInheritance<detail::Member<T>, not std::is_void_v<T>>::m_value;
    }

    operator ReturnType() {
        return value();
    }

private:
    std::coroutine_handle<promise_type> m_handle;

    friend detail::return_promise_type<ReturnType, true>;
};  // namespace IE::Core::Threading

template<typename T>
Task<T> detail::promise_type<T>::get_return_object() {
    return Task<T>{std::coroutine_handle<typename Task<T>::promise_type>::from_promise(
      *static_cast<typename Task<T>::promise_type *>(this)
    )};
}

template<typename T>
std::suspend_never detail::promise_type<T>::final_suspend() noexcept {
    {
        std::lock_guard<std::mutex> lock{*parent->m_dependentsMutex};
        for (Awaitable *dependent : parent->m_dependents) dependent->releaseDependency();
    }
    parent->m_dependents.clear();
    *parent->m_finished = true;
    parent->m_finishedNotifier->notify_all();
    return {};
}

template<typename T, bool B>
void detail::return_promise_type<T, B>::return_value(T t_value) {
    promise_type<T>::parent->m_value = t_value;
}

template<typename T>
detail::promise_type<T>::operator T() {
    if constexpr (not std::is_void_v<T>) return parent->m_value;
}
}  // namespace IE::Core::Threading
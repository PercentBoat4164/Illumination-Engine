#pragma once

#include <exception>
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

namespace IE::Core {
template<typename T>
class Stream {
    struct promise_type {
        Stream<T> *parent;

        Stream<T> get_return_object() {
            auto obj                      = Stream<T>{std::coroutine_handle<Stream<T>::promise_type>::from_promise(this)};
            obj.m_handle.promise().parent = obj;
            return obj;
        }

        std::suspend_always initial_suspend() noexcept {
            return {};
        }

        std::suspend_never final_suspend() noexcept {};

        void unhandled_exception(){
          std::rethrow_exception(std::current_exception());
        }

        std::suspend_always yield_value(T t_value) {
            parent->m_value = t_value;
            return {};
        }

        operator T() { return parent->m_value; }
    };

    explicit Stream(std::coroutine_handle<promise_type> t_handle) : m_handle(t_handle) {}

    T next() {
        m_handle.resume();
        return m_value;
    }

    T value() {
        return m_value;
    }

    operator T() {
        return value();
    }

private:
    std::coroutine_handle<promise_type> m_handle;
    T m_value;

    friend promise_type;
};
}

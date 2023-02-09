#pragma once

#include <atomic>
#include <condition_variable>
#include <coroutine>
#include <functional>

namespace IE::Core::Threading {
class BaseTask {
public:
    bool finished() {
        return *m_finished;
    }

    virtual void execute() = 0;

    virtual void wait() = 0;

    std::atomic<bool>       *m_finished{new std::atomic<bool>};
    std::condition_variable *m_finishedNotifier{new std::condition_variable};
};

template<typename T>
class Task : public BaseTask {
public:
    T value() {
        return m_value;
    }

    operator T() {
        return m_value;
    }

    T m_value;
};

template<>
class Task<void> : public BaseTask {
public:
    void value(){};
};
}  // namespace IE::Core::Threading
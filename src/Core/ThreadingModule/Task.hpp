#pragma once

#include <atomic>
#include <condition_variable>
#if defined(AppleClang)
#    include <experimental/coroutine>
#else
#    include <coroutine>
#endif
#include <functional>

namespace IE::Core::Threading {
class ResumeAfter;

class BaseTask {
public:
    virtual ~BaseTask() = default;

    [[nodiscard]] bool finished() const;

    virtual void execute() = 0;

    virtual void wait() = 0;

    std::atomic<bool>         *m_finished{new std::atomic<bool>};
    std::condition_variable   *m_finishedNotifier{new std::condition_variable};
    std::vector<ResumeAfter *> m_dependents{};
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
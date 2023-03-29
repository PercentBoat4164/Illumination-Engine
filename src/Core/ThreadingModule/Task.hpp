#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <vector>
#if defined(AppleClang)
#    include <experimental/coroutine>
#else
#    include <coroutine>
#endif

namespace IE::Core::Threading {
class Awaitable;

class BaseTask {
public:
    virtual ~BaseTask() = default;

    [[nodiscard]] bool finished() const;

    virtual void execute() = 0;

    virtual void wait() = 0;

    std::shared_ptr<std::atomic<bool>>       m_finished{std::make_shared<std::atomic<bool>>()};
    std::shared_ptr<std::condition_variable> m_finishedNotifier{std::make_shared<std::condition_variable>()};
    std::shared_ptr<std::mutex>              m_dependentsMutex{std::make_shared<std::mutex>()};
    std::vector<Awaitable *>                 m_dependents{};
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
    void value();
};
}  // namespace IE::Core::Threading
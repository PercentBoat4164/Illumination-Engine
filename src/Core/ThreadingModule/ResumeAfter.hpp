#pragma once

#include "Awaitable.hpp"
#include "Task.hpp"

#include <atomic>
#include <memory>
#include <vector>
#if defined(AppleClang)
#    include <experimental/coroutine>
#else
#    include <coroutine>
#endif

namespace IE::Core::Threading {
class ThreadPool;

class ResumeAfter : public Awaitable {
public:
    template<typename... Args>
    explicit ResumeAfter(ThreadPool *t_threadPool, Args... args) : Awaitable(t_threadPool) {
        std::vector<std::shared_ptr<BaseTask>> tasks;
        tasks.reserve(sizeof...(args));
        (..., tasks.push_back(args));
        for (const std::shared_ptr<BaseTask> &dependent : tasks) {
            if (!*dependent->m_finished) {
                std::lock_guard<std::mutex> lock(*dependent->m_dependentsMutex);
                dependent->m_dependents.emplace_back(this);
                ++*m_dependencyCount;
            }
        }
    }

    ResumeAfter(ThreadPool *t_threadPool, const std::vector<std::shared_ptr<BaseTask>> &t_tasks) :
            Awaitable(t_threadPool) {
        for (const std::shared_ptr<BaseTask> &dependent : t_tasks)
            if (!*dependent->m_finished) {
                std::lock_guard<std::mutex> lock(*dependent->m_dependentsMutex);
                dependent->m_dependents.emplace_back(this);
                ++*m_dependencyCount;
            }
    }

    bool await_ready() override;

// clang-format off
#   if defined(AppleClang)
    void await_suspend(std::experimental::coroutine_handle<> t_handle) override;
#   else
    void await_suspend(std::coroutine_handle<> t_handle) override;
#   endif
    // clang-format on

    virtual void releaseDependency();

protected:
    //clang-format off
#if defined(AppleClang)
    std::shared_ptr<std::atomic<std::experimental::coroutine_handle<>>> m_handle{
      std::make_shared<std::atomic<std::experimental::coroutine_handle<>>>()};
#else
    std::shared_ptr<std::atomic<std::coroutine_handle<>>> m_handle{
      std::make_shared<std::atomic<std::coroutine_handle<>>>()};
#endif
    // clang-format on
    std::shared_ptr<std::atomic<size_t>> m_dependencyCount{std::make_shared<std::atomic<size_t>>()};
};
}  // namespace IE::Core::Threading
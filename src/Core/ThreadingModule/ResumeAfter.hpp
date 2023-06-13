#pragma once

#include "Awaitable.hpp"
#include "BaseTask.hpp"

#include <atomic>
#include <memory>
#include <vector>
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

namespace IE::Core::Threading {
class ThreadPool;

class ResumeAfter : public Awaitable {
public:
    template<typename... Args>
    ResumeAfter(ThreadPool *t_threadPool, ThreadType t_threadType, Args... args) :
            Awaitable(t_threadPool, t_threadType) {
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

    ResumeAfter(
      ThreadPool                                   *t_threadPool,
      ThreadType                                    t_threadType,
      const std::vector<std::shared_ptr<BaseTask>> &t_tasks
    ) :
            Awaitable(t_threadPool, t_threadType) {
        for (const std::shared_ptr<BaseTask> &dependent : t_tasks)
            if (!*dependent->m_finished) {
                std::lock_guard<std::mutex> lock(*dependent->m_dependentsMutex);
                dependent->m_dependents.emplace_back(this);
                ++*m_dependencyCount;
            }
    }

    bool await_ready() override;

    void await_suspend(std::coroutine_handle<> t_handle) override;

    void releaseDependency() override;

    virtual ~ResumeAfter() = default;

protected:
    std::shared_ptr<std::atomic<std::coroutine_handle<>>> m_handle{
      std::make_shared<std::atomic<std::coroutine_handle<>>>()};
    std::shared_ptr<std::atomic<size_t>> m_dependencyCount{std::make_shared<std::atomic<size_t>>()};
};
}  // namespace IE::Core::Threading
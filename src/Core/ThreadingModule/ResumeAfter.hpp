#pragma once

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

class ResumeAfter {
public:
    template<typename... Args>
    explicit ResumeAfter(ThreadPool *t_threadPool, Args... args) :
            m_threadPool(t_threadPool) {
        std::vector<std::shared_ptr<BaseTask>> tasks;
        tasks.reserve(sizeof...(args));
        (..., tasks.push_back(args));
        for (const std::shared_ptr<BaseTask> &dependent : tasks) {
            if (!*dependent->m_finished) {
                dependent->m_dependents.push_back(this);
                ++*m_dependencyCount;
            }
        }
    }

    ResumeAfter(ThreadPool *t_threadPool, const std::vector<std::shared_ptr<BaseTask>> &t_tasks) :
            m_threadPool(t_threadPool) {
        for (const std::shared_ptr<BaseTask> &dependent : t_tasks)
            if (!*dependent->m_finished) {
                dependent->m_dependents.push_back(this);
                ++*m_dependencyCount;
            }
    }

    // Indicates the readiness of the coroutine to continue. True -> resume, False -> suspend
    bool await_ready();

#   if defined(AppleClang)
    void await_suspend(std::experimental::coroutine_handle<> t_handle);
#   else
    // Disabling clang-format because it butchers this line.
    // clang-format off
    void await_suspend(std::coroutine_handle<> t_handle);
    // clang-format on
#   endif

    void await_resume();

    void releaseDependency();

private:
    ThreadPool         *m_threadPool{};
    std::atomic<size_t> *m_dependencyCount{new std::atomic<size_t>};
#   if defined(AppleClang)
    std::atomic<std::experimental::coroutine_handle<>> *m_handle{std::atomic<std::experimental::coroutine_handle<>>};
#   else
    std::atomic<std::coroutine_handle<>> *m_handle{new std::atomic<std::coroutine_handle<>>};
#   endif
} __attribute__((aligned(32)));
}  // namespace IE::Core::Threading
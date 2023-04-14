#pragma once

#include "ResumeAfter.hpp"

#include <memory>
#include <vector>

namespace IE::Core::Threading {
class BaseTask;
class ThreadPool;

class ResumeOnMainThreadAfter : public ResumeAfter {
public:
    template<typename... Args>
    explicit ResumeOnMainThreadAfter(ThreadPool *t_threadPool, Args... args) : ResumeAfter(t_threadPool, args...) {
    }

    ResumeOnMainThreadAfter(ThreadPool *t_threadPool, const std::vector<std::shared_ptr<BaseTask>> &t_tasks) :
            ResumeAfter(t_threadPool, t_tasks) {
    }

    void releaseDependency() override;

    ~ResumeOnMainThreadAfter() = default;
};
}  // namespace IE::Core::Threading
#pragma once

#include "Awaitable.hpp"

namespace IE::Core::Threading {
class ThreadPool;

class EnsureThread : public Awaitable {
public:
    EnsureThread(ThreadPool *t_threadPool, ThreadType t_threadType);

    bool await_ready() override;

    void releaseDependency() override;

    void await_suspend(std::coroutine_handle<> t_handle) override;

    virtual ~EnsureThread() = default;
};
}  // namespace IE::Core::Threading
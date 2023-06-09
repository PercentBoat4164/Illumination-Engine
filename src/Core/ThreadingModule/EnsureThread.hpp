#pragma once

#include "Awaitable.hpp"

namespace IE::Core::Threading {
class ThreadPool;

class EnsureThread : public Awaitable {
public:
    virtual ~EnsureThread() = default;

    EnsureThread(ThreadPool *t_threadPool, ThreadType t_threadType);

    bool await_ready() override;

    void releaseDependency() override;

#if defined(AppleClang)
    void await_suspend(std::experimental::coroutine_handle<> t_handle) override;
#else
    void await_suspend(std::coroutine_handle<> t_handle) override;
#endif

protected:
    ThreadType m_type;
};
}  // namespace IE::Core::Threading
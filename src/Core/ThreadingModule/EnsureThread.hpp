#pragma once

#include "Awaitable.hpp"

namespace IE::Core::Threading {
enum ThreadType {
    IE_THREAD_TYPE_MAIN_THREAD,
    IE_THREAD_TYPE_WORKER_THREAD,
};

class ThreadPool;

class EnsureThread : public Awaitable {
public:
    virtual ~EnsureThread() = default;

    EnsureThread(ThreadPool *t_threadPool, ThreadType t_type);

    bool await_ready() override;

#if defined(AppleClang)
    void await_suspend(std::experimental::coroutine_handle<> t_handle) override;
#else
    void await_suspend(std::coroutine_handle<> t_handle) override;
#endif

protected:
    ThreadType m_type;
};
}  // namespace IE::Core::Threading
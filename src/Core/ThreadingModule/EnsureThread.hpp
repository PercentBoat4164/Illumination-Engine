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

    void await_suspend(std::coroutine_handle<> t_handle) override;


protected:
    ThreadType m_type;
};
}  // namespace IE::Core::Threading
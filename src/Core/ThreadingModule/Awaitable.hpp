#pragma once

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
enum ThreadType {
    IE_THREAD_TYPE_MAIN_THREAD,
    IE_THREAD_TYPE_WORKER_THREAD,
};
}  // namespace IE::Core::Threading

namespace IE::Core::Threading {
class ThreadPool;

class Awaitable {
public:
    // Indicates the readiness of the coroutine to continue. True -> resume, False -> suspend
    virtual bool await_ready() = 0;

    virtual void await_suspend(std::coroutine_handle<> t_handle) = 0;

    virtual void await_resume();

    virtual void releaseDependency() = 0;

protected:
    void submit(std::coroutine_handle<> t_handle);

    Awaitable(ThreadPool *t_threadPool, ThreadType t_threadType);

    ThreadPool *m_threadPool;
    ThreadType  m_threadType;
};
}  // namespace IE::Core::Threading

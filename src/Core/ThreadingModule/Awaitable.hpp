#pragma once

#if defined(AppleClang)
#    include <experimental/coroutine>
namespace std {
    using std::experimental::coroutine_handle;
    using std::experimental::suspend_always;
    using std::experimental::suspend_never;
}
#else
#    include <coroutine>
#endif

namespace IE::Core::Threading {
class ThreadPool;

class Awaitable {
public:
    // Indicates the readiness of the coroutine to continue. True -> resume, False -> suspend
    virtual bool await_ready() = 0;

    virtual void await_suspend(std::coroutine_handle<> t_handle) = 0;

    virtual void await_resume();

protected:
    explicit Awaitable(ThreadPool *t_threadPool);

    ThreadPool *m_threadPool{};
};
}  // namespace IE::Core::Threading
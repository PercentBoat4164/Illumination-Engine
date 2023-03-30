#include "EnsureThread.hpp"

#include "ThreadPool.hpp"

IE::Core::Threading::EnsureThread::EnsureThread(
  IE::Core::Threading::ThreadPool *t_threadPool,
  IE::Core::Threading::ThreadType  t_type
) :
        m_type(t_type),
        Awaitable(t_threadPool) {
}

bool IE::Core::Threading::EnsureThread::await_ready() {
    return false;
}

#if defined(AppleClang)
void IE::Core::Threading::EnsureThread::await_suspend(std::experimental::coroutine_handle<> t_handle) {
    auto func = [t_handle] {
        std::experimental::coroutine_handle<> handle{t_handle};
        handle.resume();
    };
    if (m_type == IE_THREAD_TYPE_MAIN_THREAD) m_threadPool->submitToMainThread(func);
    else if (m_type == IE_THREAD_TYPE_WORKER_THREAD) m_threadPool->submit(func);
#else
void IE::Core::Threading::EnsureThread::await_suspend(std::coroutine_handle<> t_handle) {
    if (m_type == IE_THREAD_TYPE_MAIN_THREAD) m_threadPool->submitToMainThread(t_handle);
    else if (m_type == IE_THREAD_TYPE_WORKER_THREAD) m_threadPool->submit(t_handle);
#endif
}

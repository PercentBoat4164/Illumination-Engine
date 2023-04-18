#include "EnsureThread.hpp"

#include "ThreadPool.hpp"

#include <thread>

IE::Core::Threading::EnsureThread::EnsureThread(
  IE::Core::Threading::ThreadPool *t_threadPool,
  IE::Core::Threading::ThreadType  t_type
) :
        m_type(t_type),
        Awaitable(t_threadPool) {
}

bool IE::Core::Threading::EnsureThread::await_ready() {
    return std::this_thread::get_id() == m_threadPool->mainThreadID ^ (m_type != IE_THREAD_TYPE_MAIN_THREAD);
}

void IE::Core::Threading::EnsureThread::await_suspend(std::coroutine_handle<> t_handle) {
#if defined(AppleClang)
    auto func = [t_handle] {
        std::experimental::coroutine_handle<> handle{t_handle};
        handle.resume();
    };
#else
    auto func = t_handle;
#endif
    if (m_type == IE_THREAD_TYPE_MAIN_THREAD) m_threadPool->submitToMainThread(func);
    else if (m_type == IE_THREAD_TYPE_WORKER_THREAD) m_threadPool->submit(func);
}

#include "EnsureThread.hpp"

#include "ThreadPool.hpp"

IE::Core::Threading::EnsureThread::EnsureThread(
  IE::Core::Threading::ThreadPool *t_threadPool,
  IE::Core::Threading::ThreadType  t_type
) : m_type(t_type), Awaitable(t_threadPool) {}

bool IE::Core::Threading::EnsureThread::await_ready() {
    return false;
}

#if defined(AppleClang)
void IE::Core::Threading::EnsureThread::await_suspend(std::experimental::coroutine_handle<> t_handle) {
#else
void IE::Core::Threading::EnsureThread::await_suspend(std::coroutine_handle<> t_handle) {
#endif
//    m_threadPool->submit(t_handle, m_type);
}

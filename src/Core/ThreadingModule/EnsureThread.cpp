#include "EnsureThread.hpp"

#include "ThreadPool.hpp"

#include <thread>

IE::Core::Threading::EnsureThread::EnsureThread(
  IE::Core::Threading::ThreadPool *t_threadPool,
  IE::Core::Threading::ThreadType  t_threadType
) :
        Awaitable(t_threadPool, t_threadType) {
}

bool IE::Core::Threading::EnsureThread::await_ready() {
    return std::this_thread::get_id() == m_threadPool->mainThreadID ^ (m_type != IE_THREAD_TYPE_MAIN_THREAD);
}

void IE::Core::Threading::EnsureThread::await_suspend(std::coroutine_handle<> t_handle) {
    m_threadPool->submit(m_type, t_handle);
}

void IE::Core::Threading::EnsureThread::releaseDependency() {
}

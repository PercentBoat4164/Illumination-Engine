#include "Awaitable.hpp"

#include "EnsureThread.hpp"
#include "ThreadPool.hpp"

void IE::Core::Threading::Awaitable::await_resume() {
}

void IE::Core::Threading::Awaitable::submit(std::coroutine_handle<> t_handle) {
    m_threadPool->submit(m_threadType, t_handle);
}

IE::Core::Threading::Awaitable::Awaitable(
  IE::Core::Threading::ThreadPool *t_threadPool,
  IE::Core::Threading::ThreadType  t_threadType
) :
        m_threadPool(t_threadPool),
        m_threadType(t_threadType) {
}

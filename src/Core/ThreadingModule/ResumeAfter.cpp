#include "ResumeAfter.hpp"

#include "ThreadPool.hpp"

bool IE::Core::Threading::ResumeAfter::await_ready() {
    return *m_dependencyCount == 0;
}

#if defined(AppleClang)
void IE::Core::Threading::ResumeAfter::await_suspend(std::experimental::coroutine_handle<> t_handle) {
#else
void IE::Core::Threading::ResumeAfter::await_suspend(std::coroutine_handle<> t_handle) {
#endif
    m_handle->store(t_handle);
}

void IE::Core::Threading::ResumeAfter::await_resume() {
}

void IE::Core::Threading::ResumeAfter::releaseDependency() {
    if (--*m_dependencyCount == 0) m_threadPool->submit(m_handle->load());
}

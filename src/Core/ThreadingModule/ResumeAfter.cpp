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

void IE::Core::Threading::ResumeAfter::releaseDependency() {
    // clang-format off
    if (--*m_dependencyCount == 0) {
        std::coroutine_handle<> handle{m_handle->load()};
        m_threadPool->submit([handle] {
            handle.resume();
        });
    }
    // clang-format on
}

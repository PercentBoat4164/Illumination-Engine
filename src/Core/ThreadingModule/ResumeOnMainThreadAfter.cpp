#include "ResumeOnMainThreadAfter.hpp"

#include "ThreadPool.hpp"

void IE::Core::Threading::ResumeOnMainThreadAfter::releaseDependency() {
    // clang-format off
    if (--*m_dependencyCount == 0) {
#       if defined(AppleClang)
        std::experimental::coroutine_handle<> handle{m_handle->load()};
        m_threadPool->submitToMainThread([handle] {
            std::experimental::coroutine_handle<> h{handle};
            h.resume();
        });
#       else
        m_threadPool->submitToMainThread(m_handle->load());
#       endif
    }
    // clang-format on
}

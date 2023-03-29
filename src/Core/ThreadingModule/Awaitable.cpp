#include "Awaitable.hpp"

#include "ThreadPool.hpp"

IE::Core::Threading::Awaitable::Awaitable(IE::Core::Threading::ThreadPool *t_threadPool) : m_threadPool(t_threadPool) {}

void IE::Core::Threading::Awaitable::await_resume() {}

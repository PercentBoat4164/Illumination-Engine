#include "WorkerThread.hpp"

#include "JobQueue.hpp"    // for JobQueue
#include "ThreadPool.hpp"  // for ThreadPool

#include <condition_variable>  // for condition_variable
#include <cstdint>             // for uint32_t
#include <functional>          // for function
#include <mutex>               // for mutex, unique_lock

void IE::Core::detail::WorkerThread::operator()() {
    std::function<void()> func;
    bool                  dequeued;
    while (!m_threadPool->isShutdown()) {
        {
            std::unique_lock<std::mutex> lock(m_threadPool->getConditionalMutex());
            if (m_threadPool->getQueue().empty()) m_threadPool->getConditionalLock().wait(lock);
            dequeued = m_threadPool->getQueue().dequeue(func);
        }
        if (dequeued) func();
    }
}

IE::Core::detail::WorkerThread::WorkerThread(ThreadPool *threadPool) : m_threadPool{threadPool}, m_id{m_nextId++} {
}

uint32_t IE::Core::detail::WorkerThread::m_nextId{};
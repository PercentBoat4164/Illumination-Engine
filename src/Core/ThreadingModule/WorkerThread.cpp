#include "WorkerThread.hpp"

#include "JobQueue.hpp"    // for JobQueue
#include "ThreadPool.hpp"  // for ThreadPool

#include <condition_variable>  // for condition_variable
#include <functional>          // for function
#include <mutex>               // for mutex, unique_lock

void WorkerThread::operator()() {
    std::function<void()> func;
    bool                  dequeued;
    while (!m_threadPool->m_shutdown) {
        {
            std::unique_lock<std::mutex> lock(m_threadPool->m_conditional_mutex);
            if (m_threadPool->m_queue.empty()) m_threadPool->m_conditional_lock.wait(lock);
            dequeued = m_threadPool->m_queue.dequeue(func);
        }
        if (dequeued) func();
    }
}

WorkerThread::WorkerThread(ThreadPool *threadPool) : m_threadPool{threadPool}, m_id{m_nextId++} {
}

uint32_t WorkerThread::m_nextId{0};
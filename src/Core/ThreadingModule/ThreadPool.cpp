#include "ThreadPool.hpp"

IE::Core::Worker::Worker(ThreadPool *t_threadPool) : m_threadPool(t_threadPool) {
    while (!m_threadPool->m_shutdown) {
        // Get and execute the oldest active job.
        std::shared_ptr<BaseTask> activeJob;
        if (m_threadPool->m_activeQueue.pop(activeJob)) activeJob->execute();
        ResumeAfter suspendedJob;
        while (m_threadPool->m_suspendedPool.pop(suspendedJob)) suspendedJob.resume();
        // Wait for a job to become available on the active queue.
        std::unique_lock<std::mutex> lock(m_threadPool->m_workAssignmentMutex);
        if (m_threadPool->m_activeQueue.empty() && !m_threadPool->m_shutdown)
            m_threadPool->m_workAssignmentConditionVariable.wait(lock);
        if (m_threadPool->m_shutdown) return;
    }
}

void IE::Core::ResumeAfter::await_suspend(std::coroutine_handle<> t_handle) {
    m_handle = t_handle;
    m_threadPool->m_suspendedPool.push(*this);
}

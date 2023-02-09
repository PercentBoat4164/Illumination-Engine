#include "ThreadPool.hpp"

IE::Core::Threading::Worker::Worker(ThreadPool *t_threadPool) {
    start(t_threadPool);
}

void IE::Core::Threading::Worker::start(ThreadPool *t_threadPool) {
    ThreadPool               &pool = *t_threadPool;
    std::shared_ptr<BaseTask> activeJob;
    ResumeAfter               suspendedJob;
    std::mutex                mutex;
    while (!pool.m_shutdown) {
        std::unique_lock<std::mutex> lock(mutex);
        pool.m_workAssignmentConditionVariable.wait(lock, [&] {
            return pool.m_activeQueue.pop(activeJob) || pool.m_shutdown;
        });
        if (pool.m_shutdown) break;
        activeJob->execute();
        while (pool.m_suspendedPool.pop(suspendedJob, [](ResumeAfter it) { return it.await_ready(); }))
            suspendedJob.resume();
    }
}

void IE::Core::Threading::ResumeAfter::await_suspend(std::coroutine_handle<> t_handle) {
    m_handle = t_handle;
    m_threadPool->m_suspendedPool.push(*this);
}

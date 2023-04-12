#include "ThreadPool.hpp"

#include "ResumeAfter.hpp"

IE::Core::Threading::ThreadPool::ThreadPool(size_t threads) {
    m_workers.reserve(threads);
    for (; threads > 0; --threads) m_workers.emplace_back([this] { IE::Core::Threading::Worker::start(this); });
}

void IE::Core::Threading::ThreadPool::startMainThreadLoop() {
    ThreadPool               &pool = *this;
    std::shared_ptr<BaseTask> task;
    std::mutex                mutex;
    while (!pool.m_shutdown) {
        std::unique_lock<std::mutex> lock(mutex);
        if (!pool.m_mainQueue.pop(task))
            pool.m_mainWorkAssignedNotifier.wait(lock, [&] () -> bool {
                return pool.m_mainQueue.pop(task) || pool.m_shutdown;
            });
        if (pool.m_shutdown) break;
        task->execute();
    }
}

IE::Core::Threading::ThreadPool::~ThreadPool() {
    m_shutdown = true;
    m_workAssignedNotifier.notify_all();
    for (std::thread &thread : m_workers)
        if (thread.joinable()) thread.join();
    m_mainWorkAssignedNotifier.notify_all();
}

uint32_t IE::Core::Threading::ThreadPool::getWorkerCount() {
    return m_workers.size();
}

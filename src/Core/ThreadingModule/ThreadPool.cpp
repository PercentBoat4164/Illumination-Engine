#include "ThreadPool.hpp"

#include "ResumeAfter.hpp"

#include <mutex>
#include <thread>

IE::Core::Threading::ThreadPool::ThreadPool(uint32_t t_threads) {
    m_workers.reserve(t_threads);
    for (; t_threads > 0; --t_threads)
        m_workers.emplace_back([this] { IE::Core::Threading::Worker::start(this); });
}

void IE::Core::Threading::ThreadPool::startMainThreadLoop() {
    ThreadPool               &pool = *this;
    std::shared_ptr<BaseTask> task;
    std::mutex                mutex;
    pool.mainThreadID = std::this_thread::get_id();
    while (!pool.m_mainShutdown) {
        std::unique_lock<std::mutex> lock(mutex);
        if (!pool.m_mainQueue.pop(task))
            pool.m_mainWorkAssignedNotifier.wait(lock, [&] () -> bool {
                return pool.m_mainQueue.pop(task) || pool.m_mainShutdown;
            });
        if (pool.m_mainShutdown) break;
        task->execute();
        task = nullptr;
    }

    // Release any unfinished task to the queue, and notify the remaining worker threads of its existence.
    if (task) {
        pool.m_queue.push(task);
        pool.m_workAssignedNotifier.notify_one();
    }
}

IE::Core::Threading::ThreadPool::~ThreadPool() {
    shutdown();
    for (std::thread &thread : m_workers)
        if (thread.joinable()) thread.join();
}

uint32_t IE::Core::Threading::ThreadPool::getWorkerCount() {
    return m_workers.size();
}

void IE::Core::Threading::ThreadPool::shutdown() {
    m_mainShutdown = true;
    m_mainWorkAssignedNotifier.notify_one();
    m_threadShutdownCount = getWorkerCount();
    m_workAssignedNotifier.notify_all();
}

void IE::Core::Threading::ThreadPool::setWorkerCount(uint32_t t_threads) {
    static std::mutex           mutex;
    // Ensure no other thread is trying to set the worker count. This would result in a deadlock.
    std::lock_guard<std::mutex> lock(mutex);

    int64_t dThreads = t_threads - (int64_t) getWorkerCount();
    if (dThreads == 0) return shutdown();
    if (dThreads < 0) {
        // Shutdown dThreads threads.
        m_threadShutdownCount = std::abs(dThreads);
        return m_workAssignedNotifier.notify_all();
    }

    // Add in the number of threads needed to bring the population up to the requested number.
    m_workers.reserve(t_threads);
    for (; dThreads > 0; --dThreads) m_workers.emplace_back([this] { IE::Core::Threading::Worker::start(this); });
}
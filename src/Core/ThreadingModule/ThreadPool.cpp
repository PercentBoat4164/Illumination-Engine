#include "ThreadPool.hpp"

#include "ResumeAfter.hpp"

#include <cassert>
#include <mutex>
#include <thread>

IE::Core::Threading::ThreadPool::ThreadPool(size_t t_threads) {
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
            pool.m_mainWorkAssignedNotifier.wait(lock, [&]() -> bool {
                return pool.m_mainQueue.pop(task) || pool.m_mainShutdown;
            });
        if (pool.m_mainShutdown) break;
        task->execute();
    }
}

IE::Core::Threading::ThreadPool::~ThreadPool() {
    m_shutdown = true;
    m_workAssignedNotifier.notify_all();
    for (std::thread &thread : m_workers)
        if (thread.joinable()) thread.join();
    m_mainShutdown = false;
    m_mainWorkAssignedNotifier.notify_one();
}

uint32_t IE::Core::Threading::ThreadPool::getWorkerCount() {
    return m_workers.size();
}

void IE::Core::Threading::ThreadPool::shutdown() {
    m_mainShutdown = true;
    m_mainWorkAssignedNotifier.notify_one();
    m_shutdown = true;
    m_workAssignedNotifier.notify_all();
}

void IE::Core::Threading::ThreadPool::setWorkerCount(size_t t_threads) {
    static std::mutex           mutex;
    // Ensure no other thread is trying to set the worker count. This would result in a deadlock.
    std::lock_guard<std::mutex> lock(mutex);

    assert(t_threads > 0);  // The number of threads must be at least 1.

    // Join all threads except for this one if it is a worker.
    m_shutdown = true;
    m_workAssignedNotifier.notify_all();
    std::thread::id thisThreadID{std::this_thread::get_id()};
    for (std::thread &thread : m_workers)
        if (thread.joinable() && thread.get_id() != thisThreadID) thread.join();
    erase_if(m_workers, [](std::thread &thread) { return !thread.joinable(); });
    m_shutdown = false;

    // Add in the number of threads needed to bring the population up to the requested number.
    m_workers.reserve(t_threads);
    t_threads -= m_workers.size();
    for (; t_threads > 0; --t_threads)
        m_workers.emplace_back([this] { IE::Core::Threading::Worker::start(this); });
    m_workers.shrink_to_fit();
}
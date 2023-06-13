#include "ThreadPool.hpp"

#include "EnsureThread.hpp"
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
            pool.m_mainWorkAssignedNotifier.wait(lock, [&]() -> bool {
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

std::shared_ptr<IE::Core::Threading::Task<void>>
IE::Core::Threading::ThreadPool::submit(std::coroutine_handle<> t_handle) {
    return submit(thisThreadType(), t_handle);
}

std::shared_ptr<IE::Core::Threading::Task<void>> IE::Core::Threading::ThreadPool::submit(
  IE::Core::Threading::ThreadType t_threadType,
  std::coroutine_handle<>         t_handle
) {
    return prepareAndSubmit(
      std::make_shared<Task<void>>([](std::coroutine_handle<> handle) -> Task<void> {
          co_return handle.resume();
      }(t_handle)),
      t_threadType
    );
}

void IE::Core::Threading::ThreadPool::awakenAll() {
    m_workAssignedNotifier.notify_all();
    m_mainWorkAssignedNotifier.notify_one();
}

IE::Core::Threading::EnsureThread
IE::Core::Threading::ThreadPool::ensureThread(IE::Core::Threading::ThreadType t_type) {
    return {this, t_type};
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

    int64_t threadCountDifference = t_threads - (int64_t) getWorkerCount();
    if (threadCountDifference == 0) return shutdown();
    if (threadCountDifference < 0) {
        // Shutdown threadCountDifference threads.
        m_threadShutdownCount = std::abs(threadCountDifference);
        return m_workAssignedNotifier.notify_all();
    }

    // Add in the number of threads needed to bring the population up to the requested number.
    m_workers.reserve(t_threads);
    for (; threadCountDifference > 0; --threadCountDifference)
        m_workers.emplace_back([this] { IE::Core::Threading::Worker::start(this); });
}
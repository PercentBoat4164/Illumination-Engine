#include "Worker.hpp"

#include "Task.hpp"
#include "ThreadPool.hpp"

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>

void IE::Core::Threading::Worker::start(ThreadPool *t_threadPool) {
    ThreadPool                  &pool = *t_threadPool;
    std::shared_ptr<BaseTask>    task;
    std::mutex                   mutex;
    std::unique_lock<std::mutex> lock(mutex);

    // Main working loop
    while (true) {
        // If threads are being asked to shut down, ensure that the number of threads to shut down is correctly synchronized.
        for (uint32_t n = pool.m_threadShutdownCount.load(); pool.m_threadShutdownCount > 0;)
            if (pool.m_threadShutdownCount.compare_exchange_weak(n, n - 1, std::memory_order_relaxed)) goto RELEASE_RESOURCES;
        // Wait until this thread is requested to awaken.
        if (!pool.m_queue.pop(task))
            pool.m_workAssignedNotifier.wait(lock, [&] { return pool.m_queue.pop(task) || (pool.m_threadShutdownCount > 0U); });
        // If threads are being asked to shut down, ensure that the number of threads to shut down is correctly synchronized.
        for (uint32_t n = pool.m_threadShutdownCount.load(); pool.m_threadShutdownCount > 0;)
            if (pool.m_threadShutdownCount.compare_exchange_weak(n, n - 1, std::memory_order_relaxed)) goto RELEASE_RESOURCES;
        // Execute the task, then nullify it
        if (task) {
            task->execute();
            task = nullptr;
        }
    }

    // Release any unfinished task to the queue, and notify the remaining worker threads of its existence.
    RELEASE_RESOURCES:
    if (task) {
        pool.m_queue.push(task);
        pool.m_workAssignedNotifier.notify_one();
    }
}

#include "ThreadPool.hpp"

#include "WorkerThread.hpp"  // for WorkerThread

IE::Core::ThreadPool::ThreadPool(uint8_t threadCount) : m_threads(threadCount), m_shutdown(false) {
    std::generate(
      m_threads.begin(),
      m_threads.begin() + static_cast<int8_t>(threadCount),
      [&]() -> std::thread { return std::thread(IE::Core::detail::WorkerThread(this)); }
    );
}

std::condition_variable &IE::Core::ThreadPool::getCondition() {
    return m_condition;
}

IE::Core::detail::JobQueue<std::function<void()>> &IE::Core::ThreadPool::getQueue() {
    return m_queue;
}

std::vector<std::thread> &IE::Core::ThreadPool::getThreads() {
    return m_threads;
}

std::condition_variable &IE::Core::ThreadPool::getConditionalLock() {
    return m_conditional_lock;
}

std::mutex &IE::Core::ThreadPool::getConditionalMutex() {
    return m_conditional_mutex;
}

bool IE::Core::ThreadPool::isShutdown() const {
    return m_shutdown;
}

void IE::Core::ThreadPool::shutdown() {
    m_shutdown = true;
    m_conditional_lock.notify_all();

    for (auto &m_thread : m_threads)
        if (m_thread.joinable()) m_thread.join();
}

IE::Core::ThreadPool::~ThreadPool() {
    shutdown();
}

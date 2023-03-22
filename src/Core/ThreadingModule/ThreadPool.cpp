#include "ThreadPool.hpp"

IE::Core::Threading::ThreadPool::ThreadPool(size_t threads) {
    m_workers.reserve(threads);
    for (; threads > 0; --threads) m_workers.emplace_back([this] { IE::Core::Threading::Worker::start(this); });
}

IE::Core::Threading::ResumeAfter
IE::Core::Threading::ThreadPool::resumeAfter(const std::vector<std::shared_ptr<BaseTask>> &t_tasks) {
    return ResumeAfter{this, t_tasks};
}

IE::Core::Threading::ThreadPool::~ThreadPool() {
    m_shutdown = true;
    m_workAssignmentConditionVariable.notify_all();
    for (std::thread &thread : m_workers)
        if (thread.joinable()) thread.join();
}

uint32_t IE::Core::Threading::ThreadPool::getWorkerCount() {
    return m_workers.size();
}

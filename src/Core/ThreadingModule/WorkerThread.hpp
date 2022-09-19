#pragma once

#include <cstdint>  // for uint32_t

namespace IE::Core {
class ThreadPool;
}  // namespace IE::Core

namespace IE::Core::detail {
class WorkerThread {
public:
    explicit WorkerThread(IE::Core::ThreadPool *threadPool);

    void operator()();

private:
    static uint32_t m_nextId;
    uint32_t        m_id;
    ThreadPool     *m_threadPool;
};
}  // namespace IE::Core::detail
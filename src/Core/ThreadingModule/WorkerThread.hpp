#pragma once

#include <cstdint>
#include <functional>
#include <mutex>

class ThreadPool;

class WorkerThread {
public:
    explicit WorkerThread(ThreadPool *threadPool);

    void operator()();

private:
    static uint32_t m_nextId;
    uint32_t        m_id;
    ThreadPool     *m_threadPool;
};
#pragma once

#include <algorithm>
#include <memory>
#include <mutex>
#include <queue>

/** A lock-free thread-safe FIFO queue for use in the thread pool. */
template<typename T>
class Queue {
    std::vector<T>              m_queue;
    std::shared_ptr<std::mutex> m_mutex{std::make_shared<std::mutex>()};

public:
    void push(T t_value) {
        std::unique_lock<std::mutex> lock(*m_mutex);
        m_queue.push_back(t_value);
    }

    bool pop(T &t_value) {
        std::unique_lock<std::mutex> lock(*m_mutex);
        if (m_queue.empty()) return false;
        t_value = *m_queue.begin();
        m_queue.erase(m_queue.begin());
        return true;
    }

    bool empty() {
        std::unique_lock<std::mutex> lock(*m_mutex);
        return m_queue.empty();
    }
};
#pragma once

#include <memory>
#include <mutex>
#include <vector>

/** A thread-safe FIAO pool for use in the thread pool. */
template<typename T>
class Pool {
    std::vector<T>              pool;
    std::shared_ptr<std::mutex> m_mutex{std::make_shared<std::mutex>()};

public:
    void push(T t_value) {
        std::unique_lock<std::mutex> lock(*m_mutex);
        pool.push_back(t_value);
    }

    bool pop(T &t_value) {
        std::unique_lock<std::mutex> lock(*m_mutex);
        for (auto it = pool.begin(); it != pool.end(); ++it)
            if (it->await_ready()) {
                t_value = *it;
                pool.erase(it);
                return true;
            }
        return false;
    }
};
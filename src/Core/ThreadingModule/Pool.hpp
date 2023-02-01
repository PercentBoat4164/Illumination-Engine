#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace IE::Core::Threading {
/** A thread-safe FIAO pool for use in the thread pool. */
template<typename T>
class Pool {
    std::vector<T> pool;
    std::mutex     m_mutex;

public:
    void push(T t_value) {
        std::lock_guard<std::mutex> lock(m_mutex);
        pool.push_back(t_value);
    }

    bool pop(T &t_value, std::function<bool(T)> predicate) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto it = pool.begin(); it != pool.end(); ++it)
            if (predicate(*it)) {
                t_value = *it;
                pool.erase(it);
                return true;
            }
        return false;
    }
};
}  // namespace IE::Core::Threading
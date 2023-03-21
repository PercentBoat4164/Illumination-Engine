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

    /** Pops the next value that satisfies the `t_predicate` within `t_maxDepth` elements from the beginning. */
    bool pop(T &t_value, std::function<bool(T)> t_predicate, int t_maxDepth) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto it = pool.begin(); it != pool.end(); ++it) {
            if (t_predicate(*it)) {
                t_value = *it;
                pool.erase(it);
                return true;
            }
            if (--t_maxDepth == 0) return false;
        }
        return false;
    }
};
}  // namespace IE::Core::Threading
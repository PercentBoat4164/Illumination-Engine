#pragma once

#include <condition_variable>
#include <mutex>

class MultiConditionVariable {
    std::condition_variable_any m_primaryCondition;
    std::mutex                 &m_mutex;

    explicit MultiConditionVariable(std::mutex &t_mutex) : m_mutex(t_mutex) {
    }

    template<typename... Args>
        requires requires(Args &&...t_conditions) {true;
//                     (std::is_same_v<t_conditions, std::condition_variable_any>, ...);
                 }
    void wait(Args &&...t_conditions) {
    }
};
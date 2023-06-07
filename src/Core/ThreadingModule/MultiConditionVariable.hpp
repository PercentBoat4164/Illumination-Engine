#pragma once

#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

class MultiConditionVariable {
    std::condition_variable_any m_primaryCondition;
    std::mutex                  m_mutex;

    void
    wait(std::function<bool()> t_predicate, std::initializer_list<std::condition_variable_any *> t_conditions) {
        std::vector<std::jthread> threads{0};
        for (auto *condition : t_conditions)
            threads.emplace_back([&condition, &t_predicate, this](const std::stop_token &stopToken) {
                condition->wait(m_mutex, stopToken, [&stopToken, &t_predicate] {
                    return stopToken.stop_requested() || t_predicate();
                });
                m_primaryCondition.notify_one();
            });
        m_primaryCondition.wait(m_mutex);
        for (auto &thread : threads) thread.request_stop();
    }

public:
    template<typename... Args>
    void wait(std::function<bool()> t_predicate, Args &&...t_conditions) {
        wait(t_predicate, {t_conditions...});
    }
};
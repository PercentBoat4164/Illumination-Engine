#pragma once

#include <atomic>
#include <condition_variable>
#include <vector>

namespace IE::Core::Threading {
class Awaitable;

class BaseTask {
public:
    virtual ~BaseTask() = default;

    [[nodiscard]] bool finished() const;

    virtual void execute() = 0;

    std::shared_ptr<std::atomic<bool>>           m_finished{std::make_shared<std::atomic<bool>>()};
    std::shared_ptr<std::condition_variable_any> m_finishedNotifier{
      std::make_shared<std::condition_variable_any>()};
    std::shared_ptr<std::mutex> m_dependentsMutex{std::make_shared<std::mutex>()};
    std::vector<Awaitable *>    m_dependents{};
};
}  // namespace IE::Core::Threading
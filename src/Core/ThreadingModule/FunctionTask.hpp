#pragma once

#include "Task.hpp"

#include <functional>
#include <mutex>

namespace IE::Core::Threading {
template<typename T>
class FunctionTask : public Task<T> {
public:
    FunctionTask(std::function<T()> t_baseFunction) : m_wrappedFunction(t_baseFunction) {
    }

    void execute() override {
        if constexpr (std::same_as<T, void>) m_wrappedFunction();
        else Task<T>::m_value = m_wrappedFunction();
        *(BaseTask::m_finished) = true;
        BaseTask::m_finishedNotifier->notify_all();
    }

    void wait() override {
        if (!*BaseTask::m_finished) {
            std::mutex                   mutex;
            std::unique_lock<std::mutex> lock(mutex);
            BaseTask::m_finishedNotifier->wait(lock);
        }
    }

    std::function<T()> m_wrappedFunction;
};
}  // namespace IE::Core::Threading
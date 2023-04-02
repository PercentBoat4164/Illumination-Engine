#include "Task.hpp"

bool IE::Core::Threading::BaseTask::finished() const {
    return *m_finished;
}

void IE::Core::Threading::Task<void>::value() {
}
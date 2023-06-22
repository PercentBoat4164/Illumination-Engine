#include "BaseTask.hpp"

bool IE::Core::Threading::BaseTask::finished() const {
    return *m_finished;
}
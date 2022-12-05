#include "Engine.hpp"

#include "Core/Core.hpp"

#include <utility>

IE::Core::Engine &IE::Core::Engine::operator=(const IE::Core::Engine &t_other) {
    if (this == &t_other) m_aspects = t_other.m_aspects;
    return *this;
}

IE::Core::Engine &IE::Core::Engine::operator=(IE::Core::Engine &&t_other) noexcept {
    if (this == &t_other) m_aspects = std::exchange(t_other.m_aspects, {});
    return *this;
}

bool IE::Core::Engine::finish() {
    std::unique_lock<std::mutex> lock(m_jobsMutex);
    for (std::future<void> &job : m_jobs) job.get();
    m_jobs.clear();
    return true;
}

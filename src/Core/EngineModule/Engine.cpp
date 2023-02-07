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

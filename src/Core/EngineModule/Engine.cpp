#include "Engine.hpp"

#include "Core/AssetModule/Aspect.hpp"

#include <utility>

IE::Core::Aspect *IE::Core::Engine::getAspect(const std::string &t_id) {
    auto aspect = m_aspects.find(t_id);
    if (aspect != m_aspects.end()) return aspect->second.get();
    return nullptr;
}

IE::Core::Engine &IE::Core::Engine::operator=(const IE::Core::Engine &t_other) {
    if (this == &t_other) m_aspects = t_other.m_aspects;
    return *this;
}

IE::Core::Engine &IE::Core::Engine::operator=(IE::Core::Engine &&t_other) noexcept {
    if (this == &t_other) m_aspects = std::exchange(t_other.m_aspects, {});
    return *this;
}

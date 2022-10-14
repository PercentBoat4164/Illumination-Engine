#include "Engine.hpp"

std::weak_ptr<IEAspect> IE::Core::Engine::findAspect(const std::string &filename) {
    return m_aspects.at(filename);
}

uint64_t IE::Core::Engine::m_nextId{0};
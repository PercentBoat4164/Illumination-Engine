#include "Engine.hpp"

#include <utility>

IE::Core::Engine::Engine(const std::string &t_id) {
    m_ID = t_id;
}

std::string IE::Core::Engine::getID() {
    return m_ID;
}

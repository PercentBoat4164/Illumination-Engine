#include "Engine.hpp"

std::weak_ptr<IEAspect> IE::Core::Engine::findAspect(const std::string &filename) {
    return aspects.at(filename);
}

#include "IEAsset.hpp"

#include "Core/EngineModule/Engine.hpp"

void IEAsset::addAspect(IE::Core::Engine *t_engine, const std::string &t_filename) {
    aspects.emplace_back(t_engine->createAspect(t_filename));
}

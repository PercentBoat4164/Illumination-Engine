#pragma once

#include "Core/EngineModule/Engine.hpp"

namespace IE::Script {
class Script;

class ScriptEngine : public IE::Core::Engine {
    static const std::unordered_map<
      std::string,
      std::function<std::shared_ptr<IE::Script::Script>(IE::Core::Engine *, IE::Core::File *)>>
      extensionsToLanguage;

public:
    std::shared_ptr<IE::Script::Script> createAspect(const std::string &t_id, IE::Core::File *t_resource);

    std::shared_ptr<Engine> create() override;

    bool update() override;
};
}  // namespace IE::Script

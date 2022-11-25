#pragma once

#include "Core/AssetModule/IEAspect.hpp"
#include "Core/Core.hpp"

#include <filesystem>
#include <functional>
#include <memory>
#include <unordered_map>

namespace IE::Script {
class Script : public IEAspect {
private:
    static std::unordered_map<std::string, std::function<std::shared_ptr<IE::Script::Script>(IE::Core::File *)>>
      extensionsToLanguage;

public:
    virtual void update() = 0;

    virtual void initialize() = 0;

    virtual void load() = 0;

    virtual void compile() = 0;

    static std::shared_ptr<Script> create(Core::File *t_file);
};
}  // namespace IE::Script
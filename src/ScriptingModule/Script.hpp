#pragma once

#include "Core/AssetModule/IEAspect.hpp"

#include <filesystem>
#include <functional>
#include <memory>
#include <unordered_map>

namespace IE::Script {
class Script : public IEAspect {
private:
    static std::unordered_map<std::string, std::function<std::unique_ptr<IE::Script::Script>(const std::string &)>>
      extensionsToLanguage;

public:
    virtual void update() = 0;

    virtual void initialize() = 0;

    virtual void compile() = 0;

    static std::unique_ptr<IE::Script::Script> create(const std::string &filename);
};
}  // namespace IE::Script
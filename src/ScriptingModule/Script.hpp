#pragma once

#include "Core/AssetModule/Aspect.hpp"

namespace IE::Script {
class Script : public IE::Core::Aspect {
protected:
    Script(Core::Engine *t_engine, Core::File *t_resource);

public:
    virtual void update() = 0;

    virtual void initialize() = 0;

    virtual void load() = 0;

    virtual void compile() = 0;
};
}  // namespace IE::Script
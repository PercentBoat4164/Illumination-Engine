#pragma once

#include "Core/AssetModule/Aspect.hpp"
#include "Core/EngineModule/Engine.hpp"
#include "Keyboard.hpp"

struct GLFWwindow;

namespace IE::Input {
class InputEngine : public IE::Core::Engine {
public:
    using AspectType = Keyboard;

    GLFWwindow *m_window;

    InputEngine(GLFWwindow *t_window);

    AspectType *createAspect(std::weak_ptr<Core::Asset> t_asset, const std::string &t_id) override;

    AspectType *getAspect(const std::string &t_id) override;
};
}  // namespace IE::Input
#pragma once

#include "Core/EngineModule/Engine.hpp"

class GLFWwindow;

namespace IE::Input {
class Keyboard;

class InputEngine : public IE::Core::Engine {
public:
    InputEngine(GLFWwindow *t_window);
private:
    std::weak_ptr<IEAspect> createAspect(std::weak_ptr<IEAsset> asset, const std::string &filename) override;
};
}  // namespace IE::Input
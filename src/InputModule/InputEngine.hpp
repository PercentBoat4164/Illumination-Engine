#pragma once

#include "Core/EngineModule/Engine.hpp"
#include "Keyboard.hpp"

struct GLFWwindow;

namespace IE::Input {
class InputEngine : public IE::Core::Engine {
public:
    using AspectType = Keyboard;

    GLFWwindow *m_window;

    InputEngine(GLFWwindow *t_window);

    std::shared_ptr<AspectType> createAspect(const std::string &t_id);

    std::shared_ptr<AspectType> getAspect(const std::string &t_id);
};
}  // namespace IE::Input
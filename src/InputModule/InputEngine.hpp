#pragma once

#include "Core/EngineModule/Engine.hpp"
#include "Keyboard.hpp"

class GLFWwindow;

namespace IE::Input {
class InputEngine : public IE::Core::Engine {
public:
    using AspectType = Keyboard;

    GLFWwindow *m_window;

    InputEngine(GLFWwindow *t_window);

#pragma clang diagnostic push
#pragma ide diagnostic   ignored "HidingNonVirtualFunction"
    std::shared_ptr<AspectType> createAspect(const std::string &t_id);

    std::shared_ptr<AspectType> getAspect(const std::string &t_id);
#pragma clang diagnostic pop
};
}  // namespace IE::Input
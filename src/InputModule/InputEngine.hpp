#pragma once

#include "Core/EngineModule/Engine.hpp"
#include "Keyboard.hpp"

struct GLFWwindow;

namespace IE::Input {
class InputEngine : public IE::Core::Engine {
public:
    GLFWwindow *m_window;
    Keyboard    m_keyboard;

    InputEngine(const std::string &t_ID, GLFWwindow *t_window);

    std::shared_ptr<IE::Core::Aspect> createAspect(const std::string &t_id);

    std::shared_ptr<IE::Core::Aspect> getAspect(const std::string &t_id);
};
}  // namespace IE::Input
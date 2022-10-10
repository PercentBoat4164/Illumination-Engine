#pragma once

#include "Core/Engine.hpp"

#include <vector>

class GLFWwindow;

namespace IE::Input {
class Keyboard;

class InputEngine : public IE::Core::Engine {
public:
    IE::Input::Keyboard *m_keyboard;

    InputEngine(GLFWwindow *t_window);
};
}  // namespace IE::Input
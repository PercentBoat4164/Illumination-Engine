#include "InputEngine.hpp"

#include "Keyboard.hpp"

IE::Input::InputEngine::InputEngine(GLFWwindow *t_window) {
    m_keyboard = new IE::Input::Keyboard(t_window);
}

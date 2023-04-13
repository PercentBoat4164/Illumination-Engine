#include "InputEngine.hpp"

#include "Core/Core.hpp"
#include "Keyboard.hpp"
#include <GLFW/glfw3.h>

IE::Input::InputEngine::InputEngine(const std::string &t_id, GLFWwindow *t_window) :
        Engine(t_id),
        m_window(t_window),
        m_keyboard(t_window) {
    glfwSetWindowUserPointer(m_window, this);
}

std::shared_ptr<IE::Core::Aspect> createAspect(const std::string &t_id, IE::Core::File *t_resource, IE::Core::Engine *downCastedSelf) {
    return nullptr;
}

std::shared_ptr<IE::Core::Aspect> getAspect(const std::string &t_id) {
    return nullptr;
}

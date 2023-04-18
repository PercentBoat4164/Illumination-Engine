#include "InputEngine.hpp"

#include "Core/AssetModule/Asset.hpp"
#include "Core/Core.hpp"
#include "Keyboard.hpp"

std::shared_ptr<IE::Core::Aspect> IE::Input::InputEngine::createAspect(const std::string &t_id) {
    return nullptr;
}

std::shared_ptr<IE::Core::Aspect> IE::Input::InputEngine::getAspect(const std::string &t_id) {
    return nullptr;
}

IE::Input::InputEngine::InputEngine(const std::string &t_ID, GLFWwindow *t_window) :
        Engine(t_ID),
        m_window(t_window),
        m_keyboard(t_window) {
    glfwSetWindowUserPointer(t_window, &m_keyboard);
}

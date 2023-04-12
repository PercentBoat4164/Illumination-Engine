#include "InputEngine.hpp"

#include "Core/AssetModule/Asset.hpp"
#include "Core/Core.hpp"
#include "Keyboard.hpp"

std::shared_ptr<IE::Input::InputEngine::AspectType> IE::Input::InputEngine::createAspect(const std::string &t_id) {
    return IE::Core::Engine::_createAspect<AspectType>(t_id, nullptr, this);
}

std::shared_ptr<IE::Input::InputEngine::AspectType> IE::Input::InputEngine::getAspect(const std::string &t_id) {
    return IE::Core::Engine::_getAspect<AspectType>(t_id);
}

IE::Input::InputEngine::InputEngine(GLFWwindow *t_window, const std::string &t_ID) :
        Engine(t_ID),
        m_window(t_window) {
    std::shared_ptr<IE::Input::Keyboard> keyboard = std::make_shared<IE::Input::Keyboard>(this, nullptr);
    glfwSetWindowUserPointer(t_window, keyboard.get());
    keyboard->setWindow(m_window);
    m_aspects["keyboard"] = keyboard;
}

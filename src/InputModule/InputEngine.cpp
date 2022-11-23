#include "InputEngine.hpp"

#include "Core/AssetModule/Asset.hpp"
#include "Core/Core.hpp"
#include "Keyboard.hpp"

std::shared_ptr<IE::Input::InputEngine::AspectType> IE::Input::InputEngine::createAspect(const std::string &t_id) {
    std::shared_ptr<AspectType> aspect = getAspect(t_id);
    if (!aspect) aspect = std::make_shared<AspectType>(m_window);
    return aspect;
}

std::shared_ptr<IE::Input::InputEngine::AspectType> IE::Input::InputEngine::getAspect(const std::string &t_id) {
    return std::static_pointer_cast<AspectType>(IE::Core::Engine::getAspect(t_id));
}

IE::Input::InputEngine::InputEngine(GLFWwindow *t_window) : m_window(t_window) {
    IE::Core::Core::getWindow(t_window)->inputEngine = const_cast<IE::Input::InputEngine *>(this);
    m_aspects["keyboard"]                            = std::make_shared<IE::Input::Keyboard>(t_window);
}

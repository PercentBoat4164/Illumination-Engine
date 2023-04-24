#include "InputEngine.hpp"

#include "Core/AssetModule/Aspect.hpp"
#include "Core/AssetModule/Asset.hpp"
#include "Core/Core.hpp"
#include "Keyboard.hpp"

IE::Input::InputEngine::AspectType *
IE::Input::InputEngine::createAspect(std::weak_ptr<Core::Asset> t_asset, const std::string &t_id) {
    AspectType *aspect = getAspect(t_id);
    if (!aspect) aspect = new AspectType(m_window);
    t_asset.lock()->addAspect(aspect);
    return aspect;
}

IE::Input::InputEngine::AspectType *IE::Input::InputEngine::getAspect(const std::string &t_id) {
    return static_cast<AspectType *>(IE::Core::Engine::getAspect(t_id));
}

IE::Input::InputEngine::InputEngine(GLFWwindow *t_window) : m_window(t_window) {
    IE::Core::Core::getWindow(t_window)->inputEngine = const_cast<IE::Input::InputEngine *>(this);
    m_aspects["keyboard"]                            = std::make_shared<IE::Input::Keyboard>(t_window);
}

#include "InputEngine.hpp"

#include "Core/Core.hpp"
#include "Keyboard.hpp"

std::weak_ptr<IEAspect>
IE::Input::InputEngine::createAspect(std::weak_ptr<IEAsset> asset, const std::string &filename) {
    return {};
}

IE::Input::InputEngine::InputEngine(GLFWwindow *t_window) {
    IE::Core::Core::getWindow(t_window)->inputEngine = const_cast<IE::Input::InputEngine *>(this);
    m_aspects["keyboard"]                            = std::make_shared<IE::Input::Keyboard>(t_window);
}

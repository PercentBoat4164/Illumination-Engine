#include "Keyboard.hpp"

#include "Core/Core.hpp"
#include "InputEngine.hpp"

#include <memory>

IE::Input::detail::KeyPressDescription::KeyPressDescription(
  int initialKey,
  int initialAction,
  int initialModifiers
) {
    key       = initialKey;
    scancode  = glfwGetKeyScancode(key);
    action    = initialAction;
    modifiers = initialModifiers;
}

IE::Input::detail::KeyPressDescription::KeyPressDescription(int initialKey) {
    key       = initialKey;
    scancode  = glfwGetKeyScancode(key);
    action    = GLFW_PRESS;
    modifiers = 0;
}

bool IE::Input::detail::KeyPressDescription::operator==(const IE::Input::detail::KeyPressDescription &other
) const {
    return static_cast<int>(
             (this->key == other.key) & static_cast<int>(this->scancode == other.scancode) &
             static_cast<int>(this->action == other.action) & static_cast<int>(this->modifiers == other.modifiers)
           ) != 0;
}

size_t
std::hash<IE::Input::detail::KeyPressDescription>::operator()(const IE::Input::detail::KeyPressDescription &k
) const {
    return ((((std::hash<int>()(k.key) ^ std::hash<int>()(k.scancode) >> 1) << 1) ^ std::hash<int>()(k.action) << 1
            )
            << 1) ^
      std::hash<int>()(k.modifiers);
}

IE::Input::Keyboard::Keyboard(GLFWwindow *initialWindow) {
    window = initialWindow;
    glfwSetKeyCallback(window, keyCallback);
}

void IE::Input::Keyboard::setEnqueueMethod(GLFWkeyfun function) {
    glfwSetKeyCallback(window, function);
}

void IE::Input::Keyboard::handleQueue() {
    for (size_t i{0}; i < queue.size(); ++i) {
        auto &press   = queue[i];
        auto  element = actionsOptions.find(press);
        if (element != actionsOptions.end()) {  // for each element that has a correlating action
            element->second.first(window);
            if (static_cast<int>((!element->second.second) | static_cast<int>(press.action == GLFW_RELEASE)) != 0)  // remove elements labeled to not repeat or release
                queue.erase(std::find(queue.begin(), queue.end(), press));
        }
    }
}

void IE::Input::Keyboard::editActions(
  const IE::Input::detail::KeyPressDescription             &keyPressDescription,
  const std::pair<std::function<void(GLFWwindow *)>, bool> &action
) {
    actionsOptions.erase(keyPressDescription);
    actionsOptions.insert({keyPressDescription, action});
}

void IE::Input::Keyboard::editActions(
  uint16_t                                                  key,
  const std::pair<std::function<void(GLFWwindow *)>, bool> &action
) {
    IE::Input::detail::KeyPressDescription keyPressDescription{key};
    actionsOptions.erase(keyPressDescription);
    actionsOptions.insert({keyPressDescription, action});
}

void IE::Input::Keyboard::editActions(
  uint16_t                                                  key,
  uint16_t                                                  keyAction,
  uint16_t                                                  modifiers,
  const std::pair<std::function<void(GLFWwindow *)>, bool> &action
) {
    IE::Input::detail::KeyPressDescription keyPressDescription{key, keyAction, modifiers};
    actionsOptions.erase(keyPressDescription);
    actionsOptions.insert({keyPressDescription, action});
}

void IE::Input::Keyboard::editActions(
  const IE::Input::detail::KeyPressDescription &keyPressDescription,
  const std::function<void(GLFWwindow *)>      &action
) {
    actionsOptions.erase(keyPressDescription);
    actionsOptions.insert({
      keyPressDescription,
      {action, keyPressDescription.action == GLFW_REPEAT}
    });
}

void IE::Input::Keyboard::editActions(uint16_t key, const std::function<void(GLFWwindow *)> &action, bool repeat) {
    IE::Input::detail::KeyPressDescription keyPressDescription{key};
    actionsOptions.erase(keyPressDescription);
    actionsOptions.insert({
      keyPressDescription,
      {action, repeat}
    });
}

void IE::Input::Keyboard::editActions(
  uint16_t                                 key,
  uint16_t                                 keyAction,
  uint16_t                                 modifiers,
  const std::function<void(GLFWwindow *)> &action,
  bool                                     repeat
) {
    IE::Input::detail::KeyPressDescription keyPressDescription{key, keyAction, modifiers};
    actionsOptions.erase(keyPressDescription);
    actionsOptions.insert({
      keyPressDescription,
      {action, repeat}
    });
}

void IE::Input::Keyboard::clearQueue() {
    queue.clear();
}

void IE::Input::Keyboard::keyCallback(GLFWwindow *window, int key, int scancode, int action, int modifiers) {
    auto keyboard = static_cast<IE::Input::Keyboard *>(
      static_cast<IE::Input::InputEngine *>(IE::Core::Core::getWindow(window)->inputEngine)->getAspect("keyboard")
    );  // keyboard connected to the window
    if (action == GLFW_REPEAT) {
        keyboard->queue.emplace_back(key, action);
        return;
    }
    IE::Input::detail::KeyPressDescription thisKeyPress{key, action};
    IE::Input::detail::KeyPressDescription oppositeKeyPress{
      key,
      thisKeyPress.action == GLFW_PRESS ? GLFW_RELEASE : GLFW_PRESS};
    auto oppositeKeyPressIterator = std::find(keyboard->queue.begin(), keyboard->queue.end(), oppositeKeyPress);
    if (oppositeKeyPressIterator != keyboard->queue.end()) keyboard->queue.erase(oppositeKeyPressIterator);
    keyboard->queue.push_back(thisKeyPress);
}

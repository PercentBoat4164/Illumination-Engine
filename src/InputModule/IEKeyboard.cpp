#include "IEKeyboard.hpp"

IEKeyPressDescription::IEKeyPressDescription(int initialKey, int initialAction, int initialModifiers) {
    key       = initialKey;
    scancode  = glfwGetKeyScancode(key);
    action    = initialAction;
    modifiers = initialModifiers;
}

IEKeyPressDescription::IEKeyPressDescription(int initialKey) {
    key       = initialKey;
    scancode  = glfwGetKeyScancode(key);
    action    = GLFW_PRESS;
    modifiers = 0;
}

bool IEKeyPressDescription::operator==(const IEKeyPressDescription &other) const {
    return static_cast<int>(
             (this->key == other.key) & static_cast<int>(this->scancode == other.scancode) &
             static_cast<int>(this->action == other.action) & static_cast<int>(this->modifiers == other.modifiers)
           ) != 0;
}

size_t std::hash<IEKeyPressDescription>::operator()(const IEKeyPressDescription &k) const {
    return ((((std::hash<int>()(k.key) ^ std::hash<int>()(k.scancode) >> 1) << 1) ^ std::hash<int>()(k.action) << 1
            )
            << 1) ^
      std::hash<int>()(k.modifiers);
}

IEKeyboard::IEKeyboard(GLFWwindow *initialWindow, void *initialAttachment) {
    window     = initialWindow;
    attachment = initialAttachment;
    glfwSetKeyCallback(window, keyCallback);
}

void IEKeyboard::setEnqueueMethod(GLFWkeyfun function) {
    glfwSetKeyCallback(window, function);
}

void IEKeyboard::handleQueue() {
    for (const IEKeyPressDescription &i : queue) {
        auto element = actionsOptions.find(i);
        if (element != actionsOptions.end()) {  // for each element that has a correlating action
            element->second.first(window);
            if (static_cast<int>((!element->second.second) | static_cast<int>(i.action == GLFW_RELEASE)) != 0) {  // remove elements labeled to not repeat or release
                queue.erase(std::find(queue.begin(), queue.end(), i));
            }
        }
    }
}

void IEKeyboard::editActions(
  const IEKeyPressDescription                              &keyPressDescription,
  const std::pair<std::function<void(GLFWwindow *)>, bool> &action
) {
    actionsOptions.erase(keyPressDescription);
    actionsOptions.insert({keyPressDescription, action});
}

void IEKeyboard::editActions(uint16_t key, const std::pair<std::function<void(GLFWwindow *)>, bool> &action) {
    IEKeyPressDescription keyPressDescription{key};
    actionsOptions.erase(keyPressDescription);
    actionsOptions.insert({keyPressDescription, action});
}

void IEKeyboard::editActions(
  uint16_t                                                  key,
  uint16_t                                                  keyAction,
  uint16_t                                                  modifiers,
  const std::pair<std::function<void(GLFWwindow *)>, bool> &action
) {
    IEKeyPressDescription keyPressDescription{key, keyAction, modifiers};
    actionsOptions.erase(keyPressDescription);
    actionsOptions.insert({keyPressDescription, action});
}

void IEKeyboard::editActions(
  const IEKeyPressDescription             &keyPressDescription,
  const std::function<void(GLFWwindow *)> &action
) {
    actionsOptions.erase(keyPressDescription);
    actionsOptions.insert({
      keyPressDescription,
      {action, keyPressDescription.action == GLFW_REPEAT}
    });
}

void IEKeyboard::editActions(uint16_t key, const std::function<void(GLFWwindow *)> &action, bool repeat) {
    IEKeyPressDescription keyPressDescription{key};
    actionsOptions.erase(keyPressDescription);
    actionsOptions.insert({
      keyPressDescription,
      {action, repeat}
    });
}

void IEKeyboard::editActions(
  uint16_t                                 key,
  uint16_t                                 keyAction,
  uint16_t                                 modifiers,
  const std::function<void(GLFWwindow *)> &action,
  bool                                     repeat
) {
    IEKeyPressDescription keyPressDescription{key, keyAction, modifiers};
    actionsOptions.erase(keyPressDescription);
    actionsOptions.insert({
      keyPressDescription,
      {action, repeat}
    });
}

void IEKeyboard::clearQueue() {
    queue.clear();
}

void IEKeyboard::keyCallback(GLFWwindow *window, int key, int scancode, int action, int modifiers) {
    auto keyboard = (IEKeyboard *) ((IEWindowUser *) (glfwGetWindowUserPointer(window)))
                      ->IEKeyboard;  // keyboard connected to the window
    if (action == GLFW_REPEAT) {
        keyboard->queue.emplace_back(key, action);
        return;
    }
    IEKeyPressDescription thisKeyPress{key, action};
    IEKeyPressDescription oppositeKeyPress{key, thisKeyPress.action == GLFW_PRESS ? GLFW_RELEASE : GLFW_PRESS};
    auto oppositeKeyPressIterator = std::find(keyboard->queue.begin(), keyboard->queue.end(), oppositeKeyPress);
    if (oppositeKeyPressIterator != keyboard->queue.end()) keyboard->queue.erase(oppositeKeyPressIterator);
    keyboard->queue.push_back(thisKeyPress);
}

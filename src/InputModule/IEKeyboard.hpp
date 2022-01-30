#pragma once

#ifndef GLEW_IMPLEMENTATION
#define GLEW_IMPLEMENTATION
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <functional>
#include <vector>
#include <any>
#include <string>

/**
 * @brief A class that stores the data related to a key press action.
 */
struct IeKeyPressDescription {
    uint16_t key;
    uint8_t action;
    uint8_t modifiers;
    uint16_t scancode;

    /**
     * @brief Constructs a KeyPressDescription from a key, action and modifiers.
     * @param initialKey
     * @param initialAction
     * @param initialModifiers=0
     * @returns IeKeyPressDescription
     */
    IeKeyPressDescription(int initialKey, int initialAction, int initialModifiers=0) {
        key = initialKey;
        scancode = glfwGetKeyScancode(key);
        action = initialAction;
        modifiers = initialModifiers;
    }

    /**
     * Constructs a KeyPressDescription from a key. Sets action to pressed with no modifiers.
     * @param initialKey
     */
    explicit IeKeyPressDescription(int initialKey) {
        key = initialKey;
        scancode = glfwGetKeyScancode(key);
        action = GLFW_PRESS;
        modifiers = 0;
    }

    /**
     * @brief The == operator for the IeKeyPressDescription structure.
     * @param other
     * @return true if the values of the object and argument are the same, false if not.
     */
    bool operator==(const IeKeyPressDescription& other) const {
        return (this->key == other.key) & (this->scancode == other.scancode) & (this->action == other.action) & (this->modifiers == other.modifiers);
    }
};

/**
 * @brief The hash method for the IeKeyPressDescription structure.
 * @return A hash value for an IeKeyPressDescription.
 */
template<> struct [[maybe_unused]] std::hash<IeKeyPressDescription> {
    std::size_t operator()(const IeKeyPressDescription& k) const {
        return ((((std::hash<int>()(k.key) ^ std::hash<int>()(k.scancode) >> 1) << 1) ^ std::hash<int>()(k.action) << 1) << 1) ^
        std::hash<int>()(k.modifiers);
    }
};

/**
 * @brief The Keyboard class is intended to manage keyboard event handling.
 */
class IEKeyboard {
public:
    void* attachment; // pointer to object for access through the window user pointer
    IEWindowUserPointer windowUser;

    /**
     * @brief Constructs a keyboard from a initialWindow. The initialWindow's user pointer will be set to the IeKeyboard object.
     * @param initialWindow
     * @param initialAttachment=nullptr
     * @return IeKeyboard
     */
    explicit IEKeyboard(GLFWwindow* initialWindow, void* initialAttachment=nullptr) {
        window = initialWindow;
        attachment = initialAttachment;
        windowUser = {this, ((IEWindowUserPointer *)glfwGetWindowUserPointer(window))->renderEngine};
        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, keyCallback);
    }

    /**
     * @brief Sets the queue method. Pass one of the two pre-created key event handler functions.
     * @param function
     */
    void setEnqueueMethod(GLFWkeyfun function=keyCallback) {
        glfwSetKeyCallback(window, function);
    }

    /**
     * @brief Handles all the actions indicated by the key presses logged in the queue.
     */
    void handleQueue() {
        for (const IeKeyPressDescription& i : queue) {
            auto element = actionsOptions.find(i);
            if (element != actionsOptions.end()) { // for each element that has a correlating action
                element->second.first(window);
                if (!element->second.second | (i.action == GLFW_RELEASE)) { // remove elements labeled to not repeat or release
                    queue.erase(std::find(queue.begin(), queue.end(), i));
                }
            }
        }
    }

    /**
     * @brief Adds or changes a key press to function correlation.
     * @param keyPressDescription
     * @param action
     */
    void editActions(const IeKeyPressDescription& keyPressDescription, const std::pair<std::function<void(GLFWwindow*)>, bool>& action) {
        actionsOptions.erase(keyPressDescription);
        actionsOptions.insert({keyPressDescription, action});
    }

    /**
     * @brief Adds or changes key press to function correlation.
     * @param key
     * @param action
     */
    void editActions(uint16_t key, const std::pair<std::function<void(GLFWwindow*)>, bool>& action) {
        IeKeyPressDescription keyPressDescription{key};
        actionsOptions.erase(keyPressDescription);
        actionsOptions.insert({keyPressDescription, action});
    }

    /**
     * @brief Adds or changes key press to function correlation.
     * @param key
     * @param keyAction
     * @param modifiers
     * @param action
     */
    void editActions(uint16_t key, uint16_t keyAction, uint16_t modifiers, const std::pair<std::function<void(GLFWwindow*)>, bool>& action) {
        IeKeyPressDescription keyPressDescription{key, keyAction, modifiers};
        actionsOptions.erase(keyPressDescription);
        actionsOptions.insert({keyPressDescription, action});
    }

    /**
     * @brief Adds or changes a key press to function correlation.
     * @param keyPressDescription
     * @param action
     * @param repeat
     */
    void editActions(const IeKeyPressDescription& keyPressDescription, const std::function<void(GLFWwindow*)>& action, bool repeat=true) {
        actionsOptions.erase(keyPressDescription);
        actionsOptions.insert({keyPressDescription, {action, repeat}});
    }

    /**
     * @brief Adds or changes key press to function correlation.
     * @param key
     * @param action
     * @param repeat
     */
    void editActions(uint16_t key, const std::function<void(GLFWwindow*)>& action, bool repeat=true) {
        IeKeyPressDescription keyPressDescription{key};
        actionsOptions.erase(keyPressDescription);
        actionsOptions.insert({keyPressDescription, {action, repeat}});
    }

    /**
     * @brief Adds or changes key press to function correlation.
     * @param key
     * @param keyAction
     * @param modifiers
     * @param action
     * @param repeat
     */
    void editActions(uint16_t key, uint16_t keyAction, uint16_t modifiers, const std::function<void(GLFWwindow*)>& action, bool repeat=true) {
        IeKeyPressDescription keyPressDescription{key, keyAction, modifiers};
        actionsOptions.erase(keyPressDescription);
        actionsOptions.insert({keyPressDescription, {action, repeat}});
    }

    /**
     * @brief Clears the event queue.
     */
    void clearQueue() {
        queue.clear();
    }

    /**
     * @brief Default key event handler function. Enqueues the key as pressed or released. Does not handle repeats.
     * @param window
     * @param key
     * @param scancode
     * @param action
     * @param modifiers
     */
    void static keyCallback(GLFWwindow* window, int key, int scancode, int action, int modifiers) {
        if (action == GLFW_REPEAT) {
            return;
        }
        auto keyboard = (IEKeyboard *)((IEWindowUserPointer *)(glfwGetWindowUserPointer(window)))->keyboard; // keyboard connected to the window
        IeKeyPressDescription thisKeyPress{key, action};
        IeKeyPressDescription oppositeKeyPress{key, thisKeyPress.action == GLFW_PRESS ? GLFW_RELEASE : GLFW_PRESS};
        auto oppositeKeyPressIterator = std::find(keyboard->queue.begin(), keyboard->queue.end(), oppositeKeyPress);
        if (oppositeKeyPressIterator != keyboard->queue.end()) {
            keyboard->queue.erase(oppositeKeyPressIterator);
        }
        keyboard->queue.push_back(thisKeyPress);
        #ifndef NDEBUG
        printf("Key: %i, ScanCode: %i, Action: %i, Mods: %i, KeyName: %s\n", key, scancode, action, modifiers, glfwGetKeyName(key, scancode));
        #endif
    }

private:
    GLFWwindow* window; // window this keyboard manages
    std::vector<IeKeyPressDescription> queue{}; // queue of key presses
    std::unordered_map<IeKeyPressDescription, std::pair<std::function<void(GLFWwindow*)>, bool>> actionsOptions{}; // hash table of key press description to function
};
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
    int key;
    int action;
    int modifiers;
    int scancode;
    double time{};
    std::string string;

    /**
     * @brief Constructs a KeyPressDescription from a key combination string.
     * @param keyCombination
     * @returns IeKeyPressDescription
     */
    explicit IeKeyPressDescription(const std::string& keyCombination) {
        modifiers = 0;
        scancode = 0;
        action = 0;
        key = 0;
        str(keyCombination);
    }

    /**
     * @brief Constructs a KeyPressDescription from a key, action and modifiers
     * @param initialKey
     * @param initialAction=GLFW_PRESS
     * @param initialModifiers=0
     * @returns IeKeyPressDescription
     */
    explicit IeKeyPressDescription(int initialKey, int initialScancode=0, int initialAction=GLFW_PRESS, int initialModifiers=0) {
        key = initialKey;
        scancode = initialScancode == 0 ? glfwGetKeyScancode(key) : initialScancode;
        action = initialAction;
        modifiers = initialModifiers;
        str();
    }

    /**
     * @brief Builds a key combination string from the KeyPressDescription or vice versa.
     * @param keyCombination=""
     * @return Returns an empty string if keyCombination is not empty and a key combination string if keyCombination is empty.
     */
    std::string str(const std::string& keyCombination="") {
        if (keyCombination.empty()) {
            string = std::string(modifiers & GLFW_MOD_CONTROL ? "Ctrl+" : "") + std::string(modifiers & GLFW_MOD_ALT ? "Alt+" : "") +
                     std::string(modifiers & GLFW_MOD_SHIFT ? "Shift+" : "") + std::string(modifiers & GLFW_MOD_SUPER ? "Super+" : "") +
                     std::to_string(key) + std::string(action == GLFW_RELEASE ? " Released" : "") + std::string(action == GLFW_PRESS ? " Pressed" : "") +
                     std::string(action == GLFW_REPEAT ? " Repeat" : "");
            return string;
        } else {
            if (keyCombination.find("Ctrl+") != -1) {
                modifiers |= GLFW_MOD_CONTROL;
            }
            if (keyCombination.find("Alt+") != -1) {
                modifiers |= GLFW_MOD_ALT;
            }
            if (keyCombination.find("Shift+") != -1) {
                modifiers |= GLFW_MOD_SHIFT;
            }
            if (keyCombination.find("Super+") != -1) {
                modifiers |= GLFW_MOD_SUPER;
            }
            if (keyCombination.find(" Released") != -1) {
                action = GLFW_RELEASE;
            }
            if (keyCombination.find(" Pressed") != -1) {
                action = GLFW_PRESS;
            }
            if (keyCombination.find(" Repeat") != -1) {
                action = GLFW_REPEAT;
            }
            if (keyCombination.find("+") != -1) {
                key = std::stoi(keyCombination.substr(keyCombination.find_last_of('+'), keyCombination.find_last_of(' ') - 1));
            }
            scancode = glfwGetKeyScancode(key);
            return "";
        }
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
class IeKeyboard {
public:
    void* attachment; // pointer to object for access through the window user pointer

    /**
     * @brief Constructs a keyboard from a initialWindow. The initialWindow's user pointer will be set to the IeKeyboard object.
     * @param initialWindow
     * @param initialAttachment=nullptr
     * @return IeKeyboard
     */
    explicit IeKeyboard(GLFWwindow* initialWindow, void* initialAttachment=nullptr) {
        window = initialWindow;
        attachment = initialAttachment;
        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, triActionKeyCallback);
    }

    /**
     * @brief Sets the queue method. Pass one of the two pre-created key event handler functions.
     * @param function
     */
    void setEnqueueMethod(GLFWkeyfun function=triActionKeyCallback) {
        glfwSetKeyCallback(window, function);
    }

    /**
     * @brief Handles all the actions indicated by the key presses logged in the queue.
     */
    void handleQueue() {
        for (const IeKeyPressDescription& i : queue) {
            if (actionsOptions.count(i) != 0) {
                actionsOptions[i](window);
            }
        }
        queue.clear();
    }

    /**
     * @brief Adds or changes a key press to function correlation.
     * @param keyPressDescription
     * @param action
     */
    void editActions(const IeKeyPressDescription& keyPressDescription, const std::function<void(GLFWwindow*)>& action) {
        actionsOptions.erase(keyPressDescription);
        actionsOptions.insert({keyPressDescription, action});
    }

/**
 * @brief Default key event handler function. Enqueues the key as pressed or released.
 * @param window
 * @param key
 * @param scancode
 * @param action
 * @param modifiers
 */
void static dualActionKeyCallback(GLFWwindow* window, int key, int scancode, int action, int modifiers) {
    auto keyboard = static_cast<IeKeyboard*>(glfwGetWindowUserPointer(window)); // keyboard connected to the window
    keyboard->queue.emplace_back(key, scancode, action > GLFW_RELEASE ? GLFW_PRESS : GLFW_RELEASE, modifiers);
    #ifndef NDEBUG
    printf("Key: %i, ScanCode: %i, Action: %i, Mods: %i, KeyName: %s\n", key, scancode, action, modifiers, glfwGetKeyName(key, scancode));
    #endif
}

/**
 * @brief Default key event handler function. Enqueues the action it receives
 * @param window
 * @param key
 * @param scancode
 * @param action
 * @param modifiers
 */
void static triActionKeyCallback(GLFWwindow* window, int key, int scancode, int action, int modifiers) {
    auto keyboard = static_cast<IeKeyboard*>(glfwGetWindowUserPointer(window)); // keyboard connected to the window
    keyboard->queue.emplace_back(key, scancode, action, modifiers);
    #ifndef NDEBUG
    printf("Key: %i, ScanCode: %i, Action: %i, Mods: %i, KeyName: %s\n", key, scancode, action, modifiers, glfwGetKeyName(key, scancode));
    #endif
}

private:
    GLFWwindow* window; // window this keyboard manages
    std::vector<IeKeyPressDescription> queue{}; // queue of key presses
    std::unordered_map<IeKeyPressDescription, std::function<void(GLFWwindow*)>> actionsOptions{}; // hash table of key press description to function
};
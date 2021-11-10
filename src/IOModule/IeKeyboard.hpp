#pragma once

#ifndef GLEW_IMPLEMENTATION
#define GLEW_IMPLEMENTATION
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>

#include <functional>
#include <vector>
#include <cstdio>
#include <any>
#include <string>

/**
 * @brief A class that stores the data related to a key press action.
 */
struct IeKeyPressDescription {
    int key{};
    int action{};
    int modifiers{};
    std::string string{};

    /**
     * @brief Constructs a KeyPressDescription from a key combination string.
     * @param keyCombination
     * @returns IeKeyPressDescription
     */
    explicit IeKeyPressDescription(const std::string& keyCombination) {
        str(keyCombination);
    }

    /**
     * @brief Constructs a KeyPressDescription from a key, action and modifiers
     * @param initialKey
     * @param initialAction
     * @param initialModifiers
     * @returns IeKeyPressDescription
     */
    explicit IeKeyPressDescription(int initialKey, int initialAction, int initialModifiers) {
        key = initialKey;
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
            const char* keyName = glfwGetKeyName(key, 0);
            string = std::string(modifiers & GLFW_MOD_CONTROL ? "Ctrl+" : "") + std::string(modifiers & GLFW_MOD_ALT ? "Alt+" : "") +
                     std::string(modifiers & GLFW_MOD_SHIFT ? "Shift+" : "") + std::string(modifiers & GLFW_MOD_SUPER ? "Super+" : "") +
                     std::string(keyName) + std::string(action == 0 ? " Released" : "") + std::string(action == 1 ? " Pressed" : "") +
                     std::string(action == 2 ? " Repeat" : "");
            return string;
        } else {
            if (keyCombination.find("Ctrl+")) {
                modifiers |= GLFW_MOD_CONTROL;
            }
            if (keyCombination.find("Alt+")) {
                modifiers |= GLFW_MOD_ALT;
            }
            if (keyCombination.find("Shift+")) {
                modifiers |= GLFW_MOD_SHIFT;
            }
            if (keyCombination.find("Super+")) {
                modifiers |= GLFW_MOD_SUPER;
            }
            if (keyCombination.find(" Released")) {
                action = 0;
            }
            if (keyCombination.find(" Pressed")) {
                action = 1;
            }
            if (keyCombination.find(" Repeat")) {
                action = 2;
            }
            key = std::stoi(keyCombination.substr(keyCombination.find_last_of('+'), keyCombination.find_last_of(' ') - 1));
            return "";
        }
    }

    /**
     * @brief The == operator for the IeKeyPressDescription structure.
     * @param other
     * @return true if the values of the object and argument are the same, false if not.
     */
    bool operator==(const IeKeyPressDescription& other) const {
        return (this->key == other.key) & (this->action == other.action) & (this->modifiers == other.modifiers);
    }
};

/**
 * @brief The hash method for the IeKeyPressDescription structure.
 * @return Returns a hash value for an IeKeyPressDescription.
 */
template<> struct std::hash<IeKeyPressDescription> {
    std::size_t operator()(const IeKeyPressDescription& k) const {
        return ((std::hash<int>()(k.key) ^ std::hash<int>()(k.action) >> 1) << 1) ^ std::hash<int>()(k.modifiers) << 1;
    }
};

/**
 * @brief The Keyboard class is intended to manage keyboard event handling.
 */
class IeKeyboard {
public:

    /**
     * @brief Constructs a keyboard from a window. The window's user pointer will be set to the IeKeyboard object.
     * @param window
     * @return IeKeyboard
     */
    explicit IeKeyboard(GLFWwindow* window) {
        glfwSetWindowUserPointer(window, this);
        glfwSetKeyCallback(window, keyCallback);
    }

    /**
     * @brief Handles all the actions indicated by the key presses logged in the queue.
     */
    void handleQueue() {
        for (const IeKeyPressDescription& i : queue) {
            if (actionsOptions.count(i) != 0) {
                actionsOptions[i]();
            }
        }
        queue.clear();
    }

    /**
     * @brief Adds or changes a key press to function correlation.
     * @param keyPressDescription
     * @param action
     */
    void editActions(const IeKeyPressDescription& keyPressDescription, const std::function<void()>& action) {
        actionsOptions.erase(keyPressDescription);
        actionsOptions.insert({keyPressDescription, action});
    }

private:
    std::vector<IeKeyPressDescription> queue{}; // queue of key presses
    std::unordered_map<IeKeyPressDescription, std::function<void()>> actionsOptions{}; // hash table of scancodes to functions

    /**
     * @brief Default key event handler function.
     * @param window
     * @param key
     * @param scancode
     * @param action
     * @param modifiers
     */
    void static keyCallback(GLFWwindow* window, int key, int scancode, int action, int modifiers) {
        auto keyboard = static_cast<IeKeyboard*>(glfwGetWindowUserPointer(window)); // an IeKeyboard object that is connected to the window
        if (action != 2) { // if the key is not being held down
            keyboard->queue.emplace_back(key, action, modifiers);
        }
        #ifndef NDEBUG
        printf("Key: %i, ScanCode: %i, Action: %i, Mods: %i, KeyName: %s\n", key, scancode, action, modifiers, glfwGetKeyName(key, scancode));
        #endif
    }
};
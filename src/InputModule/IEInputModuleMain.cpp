#include "IeKeyboard.hpp"

#include <iostream>


/**
 * @brief Is responsible for handling a "x" key press.
 */
void handleXKey(GLFWwindow* window) {
    std::cout << "Chicken!" << std::endl;
    std::flush(std::cout);
}

/**
 * @brief Is responsible for handling a "w" key press. Demonstrates the capabilities and flexibility of this system.
 */
void handleWKey(GLFWwindow* window) {
    auto keyboard = static_cast<IeKeyboard*>(glfwGetWindowUserPointer(window)); // keyboard controlling the window
    keyboard->editActions(IeKeyPressDescription(GLFW_KEY_W), handleXKey, false);
    std::cout << "Wow-zah!" << std::endl;
    std::flush(std::cout);
}


/**
 * @brief Code for testing the InputModule will go here.
 */
int main(int argc, char **argv) {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(800, 600, "", nullptr, nullptr);
    IeKeyboard keyboard{window};
    keyboard.editActions(IeKeyPressDescription(GLFW_KEY_X), handleXKey, false);
    keyboard.editActions(IeKeyPressDescription(GLFW_KEY_W), handleWKey, false);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        keyboard.handleQueue();
    }
}
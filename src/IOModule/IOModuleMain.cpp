#include "IeKeyboard.hpp"

#include <iostream>


/**
 * @brief Is responsible for handling a "w" key press.
 *
 * @return double 3
 */
static void handleWKey() {
    std::cout << "Wow-zah!" << std::endl;
    std::flush(std::cout);
}


/**
 * @brief Code for testing the IOModule will go here.
 */
int main(int argc, char **argv) {
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(800, 600, "", nullptr, nullptr);
    IeKeyboard keyboard{window};
    IeKeyPressDescription keyPressDescription{GLFW_KEY_W, 1, 0};
    std::cout << keyPressDescription.str().c_str() << std::endl;
    keyboard.editActions(keyPressDescription, handleWKey);
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        keyboard.handleQueue();
    }
}
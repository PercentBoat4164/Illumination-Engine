#include "GraphicsModule/Vulkan/vulkanRenderEngine.hpp"
#include "InputModule/IeKeyboard.hpp"

/**
 * @brief A key callback function. Moves the camera forward.
 * @param window
 */
void moveCameraForward(GLFWwindow* window) {
    auto renderEngine = static_cast<VulkanRenderEngine *>(static_cast<IeKeyboard *>(glfwGetWindowUserPointer(window))->attachment);
    renderEngine->camera.position = glm::normalize(renderEngine->camera.position + renderEngine->camera.front * renderEngine->frameTime);
}

/**
 * @brief A key callback function. Moves the camera backward.
 * @param window
 */
void moveCameraBackward(GLFWwindow* window) {
    auto renderEngine = static_cast<VulkanRenderEngine *>(static_cast<IeKeyboard *>(glfwGetWindowUserPointer(window))->attachment);
    renderEngine->camera.position = glm::normalize(renderEngine->camera.position - renderEngine->camera.front * renderEngine->frameTime);
}

/**
 * @brief A key callback function. Moves the camera right.
 * @param window
 */
void moveCameraRight(GLFWwindow* window) {
    auto renderEngine = static_cast<VulkanRenderEngine *>(static_cast<IeKeyboard *>(glfwGetWindowUserPointer(window))->attachment);
    renderEngine->camera.position = glm::normalize(renderEngine->camera.position + renderEngine->camera.right * renderEngine->frameTime);
}

/**
 * @brief A key callback function. Moves the camera left.
 * @param window
 */
void moveCameraLeft(GLFWwindow* window) {
    auto renderEngine = static_cast<VulkanRenderEngine *>(static_cast<IeKeyboard *>(glfwGetWindowUserPointer(window))->attachment);
    renderEngine->camera.position = glm::normalize(renderEngine->camera.position - renderEngine->camera.right * renderEngine->frameTime);
}

/**
 * @brief Code for testing the entire engine with all its modular parts goes here.
 */
int main(int argc, char **argv) {
    VulkanRenderEngine renderEngine{};
    VulkanRenderable cube{&renderEngine.renderEngineLink, "res/Models/AncientStatue/ancientStatue.obj"};
    renderEngine.loadRenderable(&cube);
    IeKeyboard keyboard{renderEngine.window};
    keyboard.attachment = &renderEngine;
    keyboard.editActions(IeKeyPressDescription(GLFW_KEY_W), moveCameraForward);
    keyboard.editActions(IeKeyPressDescription(GLFW_KEY_A), moveCameraLeft);
    keyboard.editActions(IeKeyPressDescription(GLFW_KEY_S), moveCameraBackward);
    keyboard.editActions(IeKeyPressDescription(GLFW_KEY_D), moveCameraRight);
    keyboard.setEnqueueMethod(IeKeyboard::dualActionKeyCallback);
    while (renderEngine.update()) {
        glfwPollEvents();
        keyboard.handleQueue();
    }
}
#include "GraphicsModule/OpenGL/openglRenderEngine.hpp"
#include "InputModule/IeKeyboard.hpp"
#include "MotionEngine/RouteTypes/KinematicsRoute.hpp"

/**
 * @brief A key callback function. Moves the camera forward.
 * @param window
 */
void moveCameraForward(GLFWwindow* window) {
    auto renderEngine = static_cast<OpenGLRenderEngine *>(static_cast<IeKeyboard *>(glfwGetWindowUserPointer(window))->attachment);
    renderEngine->camera.position = renderEngine->camera.position + renderEngine->camera.front * renderEngine->frameTime *
            renderEngine->camera.speed;
}

/**
 * @brief A key callback function. Moves the camera backward.
 * @param window
 */
void moveCameraBackward(GLFWwindow* window) {
    auto renderEngine = static_cast<OpenGLRenderEngine *>(static_cast<IeKeyboard *>(glfwGetWindowUserPointer(window))->attachment);
    renderEngine->camera.position = renderEngine->camera.position - renderEngine->camera.front * renderEngine->frameTime *
            renderEngine->camera.speed;
}

/**
 * @brief A key callback function. Moves the camera right.
 * @param window
 */
void moveCameraRight(GLFWwindow* window) {
    auto renderEngine = static_cast<OpenGLRenderEngine *>(static_cast<IeKeyboard *>(glfwGetWindowUserPointer(window))->attachment);
    renderEngine->camera.position = renderEngine->camera.position + renderEngine->camera.right * renderEngine->frameTime *
            renderEngine->camera.speed;
}

/**
 * @brief A key callback function. Moves the camera left.
 * @param window
 */
void moveCameraLeft(GLFWwindow* window) {
    auto renderEngine = static_cast<OpenGLRenderEngine *>(static_cast<IeKeyboard *>(glfwGetWindowUserPointer(window))->attachment);
    renderEngine->camera.position = renderEngine->camera.position - renderEngine->camera.right * renderEngine->frameTime *
            renderEngine->camera.speed;
}

/**
 * @brief A key callback function. Moves the camera up.
 * @param window
 */
void moveCameraUp(GLFWwindow* window) {
    auto renderEngine = static_cast<OpenGLRenderEngine *>(static_cast<IeKeyboard *>(glfwGetWindowUserPointer(window))->attachment);
    renderEngine->camera.position = renderEngine->camera.position + renderEngine->camera.up * renderEngine->frameTime * renderEngine->camera.speed;
}

/**
 * @brief A key callback function. Moves the camera down.
 * @param window
 */
void moveCameraDown(GLFWwindow* window) {
    auto renderEngine = static_cast<OpenGLRenderEngine *>(static_cast<IeKeyboard *>(glfwGetWindowUserPointer(window))->attachment);
    renderEngine->camera.position = renderEngine->camera.position - renderEngine->camera.up * renderEngine->frameTime * renderEngine->camera.speed;
}

/**
 * @brief A key callback function. Moves the camera slower.
 * @param window
 */
void moveSlower(GLFWwindow* window);

/**
 * @brief A key callback function. Moves the camera faster.
 * @param window
 */
void moveFaster(GLFWwindow* window) {
    auto keyboard = static_cast<IeKeyboard *>(glfwGetWindowUserPointer(window));
    auto renderEngine = static_cast<OpenGLRenderEngine *>(keyboard->attachment);
    renderEngine->camera.speed *= 3;
}

void moveSlower(GLFWwindow* window) {
    auto keyboard = static_cast<IeKeyboard *>(glfwGetWindowUserPointer(window));
    auto renderEngine = static_cast<OpenGLRenderEngine *>(keyboard->attachment);
    renderEngine->camera.speed /= 3;
}

void pause(GLFWwindow* window) {
    auto keyboard = static_cast<IeKeyboard *>(glfwGetWindowUserPointer(window));
    auto renderEngine = static_cast<OpenGLRenderEngine *>(keyboard->attachment);
    renderEngine->paused ^= true;
}
/**
 * @brief A key callback function. Resets the camera position and keyboard queue.
 * @param window
 */
void resetAll(GLFWwindow* window) {
    auto keyboard = static_cast<IeKeyboard *>(glfwGetWindowUserPointer(window));
    auto renderEngine = static_cast<OpenGLRenderEngine *>(keyboard->attachment);
    keyboard->clearQueue();
    renderEngine->camera = OpenGLCamera{};
    renderEngine->camera.linkedRenderEngine = &renderEngine->renderEngineLink;
}

/**
 * @brief Code for testing the entire engine with all its modular parts goes here.
 */
int main(int argc, char **argv) {
    OpenGLRenderEngine renderEngine{};
    OpenGLRenderable sphere{&renderEngine.renderEngineLink, "res/Models/Sphere/sphere.obj"};
    sphere.position = {0, -3, 0};
    glm::quat rotInit = glm::quat(glm::vec3(M_PI / 4, M_PI / 4, M_PI / 4));
    glm::quat rotV = glm::quat(glm::vec3(M_PI / 4, 0.0f, 0.0f));
    KinematicsBody sphereBody(1, glm::vec3(0, -3, 0), rotInit, glm::vec3(0, 0, 0), rotV, glm::vec3(0, 0, -9.8));
    KinematicsRoute sphereRoute;
    sphereRoute.setBody(&sphereBody);
    renderEngine.loadRenderable(&sphere);
    IeKeyboard keyboard{renderEngine.window};
    keyboard.attachment = &renderEngine;
    keyboard.editActions(GLFW_KEY_W, moveCameraForward);
    keyboard.editActions(GLFW_KEY_A, moveCameraLeft);
    keyboard.editActions(GLFW_KEY_S, moveCameraBackward);
    keyboard.editActions(GLFW_KEY_D, moveCameraRight);
    keyboard.editActions(GLFW_KEY_SPACE, moveCameraUp);
    keyboard.editActions(GLFW_KEY_LEFT_SHIFT, moveCameraDown);
    keyboard.editActions({GLFW_KEY_LEFT_CONTROL, GLFW_PRESS}, moveFaster, false);
    keyboard.editActions({GLFW_KEY_LEFT_CONTROL, GLFW_RELEASE}, moveSlower, false);
    keyboard.editActions(GLFW_KEY_HOME, resetAll, false);
    keyboard.editActions(GLFW_KEY_P, pause, false);
    while (renderEngine.update()) {
        glfwPollEvents();
        keyboard.handleQueue();
        if(!renderEngine.paused) {
            sphereRoute.step(0.01666666f);
            sphere.position = sphereBody.position;
            sphere.rotation = glm::eulerAngles(sphereBody.rotation) * (180.0f / float(M_PI));
            printf("Pos: %f, Rot: %f, V: %f\n", sphere.position.z, sphere.rotation.x, sphereBody.velocity.z);
            if(sphereBody.position.z < -1 && sphereBody.velocity.z < 0) {
                sphereBody.velocity.z *= -1;
            }
        }
    }
}
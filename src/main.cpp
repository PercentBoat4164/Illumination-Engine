#include "GraphicsModule/IERenderEngine.cpp"
#include "InputModule/IEKeyboard.hpp"
#include "Core/AssetModule/IEAsset.hpp"
#include "Core/FileSystemModule/IEFileSystem.hpp"

int main() {
    IERenderEngine renderEngine{};
    IEKeyboard keyboard{renderEngine.window};
    keyboard.editActions(GLFW_KEY_W, [&](GLFWwindow*) { renderEngine.camera.position += renderEngine.camera.front * renderEngine.frameTime * renderEngine.camera.speed; });
    keyboard.editActions(GLFW_KEY_A, [&](GLFWwindow*) { renderEngine.camera.position -= renderEngine.camera.right * renderEngine.frameTime * renderEngine.camera.speed; });
    keyboard.editActions(GLFW_KEY_S, [&](GLFWwindow*) { renderEngine.camera.position -= renderEngine.camera.front * renderEngine.frameTime * renderEngine.camera.speed; });
    keyboard.editActions(GLFW_KEY_D, [&](GLFWwindow*) { renderEngine.camera.position += renderEngine.camera.right * renderEngine.frameTime * renderEngine.camera.speed; });
    keyboard.editActions(GLFW_KEY_SPACE, [&](GLFWwindow*) { renderEngine.camera.position += renderEngine.camera.up * renderEngine.frameTime * renderEngine.camera.speed; });
    keyboard.editActions(GLFW_KEY_LEFT_SHIFT, [&](GLFWwindow*) { renderEngine.camera.position -= renderEngine.camera.up * renderEngine.frameTime * renderEngine.camera.speed; });
    keyboard.editActions({GLFW_KEY_LEFT_CONTROL, GLFW_PRESS}, [&](GLFWwindow*) { renderEngine.camera.speed *= 6; });
    keyboard.editActions({GLFW_KEY_LEFT_CONTROL, GLFW_RELEASE}, [&](GLFWwindow*) { renderEngine.camera.speed /= 6; });
    keyboard.editActions({GLFW_KEY_F11, GLFW_PRESS}, [&](GLFWwindow*) { renderEngine.handleFullscreenSettingsChange(); });
    IEWindowUser windowUser{&renderEngine, &keyboard};
    glfwSetWindowUserPointer(renderEngine.window, &windowUser);
    IEAsset asset{};
    asset.addAspect(new IERenderable(&renderEngine, "res/Models/Cube/cube.obj"));
    renderEngine.addAsset(&asset);
    asset.position = {0.0F, -2.0F, 0.0F};
    renderEngine.camera.position = {0.0F, 2.0F, 0.0F};
    while (renderEngine.update()) {
        glfwPollEvents();
        keyboard.handleQueue();
    }
}
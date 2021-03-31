#include <iostream>
#include "Settings.hpp"
#include "AssetLinking.hpp"
#include "Asset.hpp"
#include "VulkanRenderEngine.hpp"
#include "OpenGLRenderEngine.hpp"


void cursorCallback(GLFWwindow *window, double xpos, double ypos) {
    auto vulkanRenderEngine = static_cast<VulkanRenderEngine *>(glfwGetWindowUserPointer(window));
    xpos *= vulkanRenderEngine->camera.mouseSensitivity;
    ypos *= vulkanRenderEngine->camera.mouseSensitivity;
    vulkanRenderEngine->camera.yaw -= (float)xpos;
    vulkanRenderEngine->camera.pitch -= (float)ypos;
    if (vulkanRenderEngine->camera.pitch > 89.0f) { vulkanRenderEngine->camera.pitch = 89.0f; }
    if (vulkanRenderEngine->camera.pitch < -89.0f) { vulkanRenderEngine->camera.pitch = -89.0f; }
    glfwSetCursorPos(window, 0, 0);
}

int main(int argc, char **argv) {
    std::cout << "'v': Run Vulkan render engine\n'o': Run OpenGL render engine\n'p': Run Physics engine\n";
    std::string selection;
    char input;
    if (argc > 1) { input = *argv[1]; } else {
        std::cin >> selection;
        input = selection[0];
    }
    if (input == 'v') {
        try {
            VulkanRenderEngine renderEngine = VulkanRenderEngine();
            renderEngine.camera.position = {0, 0, 2};
            Asset ancientStatue = Asset("models/vikingRoom.obj", {"models/vikingRoom.png"}, {"shaders/vertexShader.vert", "shaders/fragmentShader.frag"}, &renderEngine.settings, {0, 0, 0}, {0, 0, 0}, {5, 5, 5});
            renderEngine.uploadAsset(&ancientStatue);
            double lastTab{0};
            double lastF2{0};
            while (renderEngine.update()) {
                //Process inputs
                glfwPollEvents();
                float velocity = renderEngine.frameTime * renderEngine.camera.movementSpeed;
                if (glfwGetKey(renderEngine.window, GLFW_KEY_F1)) {
                    for (Asset *asset : renderEngine.assets) { asset->reloadAsset(); }
                    renderEngine.updateSettings(renderEngine.settings, false);
                }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_F2) & (glfwGetTime() - lastF2 > .2)) {
                    Settings newSettings{};
                    newSettings = renderEngine.settings;
                    newSettings.fullscreen = !renderEngine.settings.fullscreen;
                    renderEngine.updateSettings(newSettings, true);
                    lastF2 = glfwGetTime();
                }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_1)) {
                    renderEngine.settings.msaaSamples = VK_SAMPLE_COUNT_1_BIT;
                    renderEngine.updateSettings(renderEngine.settings, true);
                }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_8)) {
                    renderEngine.settings.msaaSamples = VK_SAMPLE_COUNT_8_BIT;
                    renderEngine.updateSettings(renderEngine.settings, true);
                }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_W)) { renderEngine.camera.position += renderEngine.camera.front * velocity; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_A)) { renderEngine.camera.position -= renderEngine.camera.right * velocity; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_S)) { renderEngine.camera.position -= renderEngine.camera.front * velocity; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_D)) { renderEngine.camera.position += renderEngine.camera.right * velocity; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_LEFT_SHIFT)) { renderEngine.camera.position -= renderEngine.camera.up * velocity; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_SPACE)) { renderEngine.camera.position += renderEngine.camera.up * velocity; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_TAB) & (glfwGetTime() - lastTab > .2)) {
                    int mode{glfwGetInputMode(renderEngine.window, GLFW_CURSOR)};
                    if (mode == GLFW_CURSOR_DISABLED) {
                        glfwSetInputMode(renderEngine.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                        glfwSetCursorPosCallback(renderEngine.window, nullptr);
                    }
                    if (mode == GLFW_CURSOR_NORMAL) {
                        glfwSetInputMode(renderEngine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                        glfwSetCursorPosCallback(renderEngine.window, cursorCallback);
                    }
                    lastTab = glfwGetTime();
                }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_ESCAPE)) { glfwSetWindowShouldClose(renderEngine.window, 1); }
            }
            ancientStatue.destroy();
            renderEngine.cleanUp();
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    else if (input == 'o') {
        try {
            OpenGLRenderEngine renderEngine = OpenGLRenderEngine();
            while (renderEngine.update() != 1) {
                glfwPollEvents();
            }
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
}
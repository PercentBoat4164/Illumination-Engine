#include <iostream>
#include "src/GraphicsEngine/Settings.hpp"
#include "src/GraphicsEngine/Vulkan/Asset.hpp"
#include "src/GraphicsEngine/Vulkan/VulkanRenderEngineRasterizer.hpp"
#include "src/GraphicsEngine/OpenGL/OpenGLRenderEngine.hpp"

void cursorCallback(GLFWwindow *window, double xOffset, double yOffset) {
    auto vulkanRenderEngine = static_cast<VulkanRenderEngineRasterizer *>(glfwGetWindowUserPointer(window));
    xOffset *= vulkanRenderEngine->camera.mouseSensitivity;
    yOffset *= vulkanRenderEngine->camera.mouseSensitivity;
    vulkanRenderEngine->camera.yaw -= (float) xOffset;
    vulkanRenderEngine->camera.pitch -= (float) yOffset;
    if (vulkanRenderEngine->camera.pitch > 89.99f) { vulkanRenderEngine->camera.pitch = 89.99f; }
    if (vulkanRenderEngine->camera.pitch < -89.99f) { vulkanRenderEngine->camera.pitch = -89.99f; }
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
            VulkanRenderEngineRasterizer renderEngine = VulkanRenderEngineRasterizer();
            renderEngine.camera.position = {0, 0, 2};
            Asset vikingRoom = Asset("models/vikingRoom.obj", {"models/vikingRoom.png"}, {"shaders/vertexShader.vert", "shaders/fragmentShader.frag"}, &renderEngine.settings, {0, 0, 0}, {0, 0, 0}, {5, 5, 5});
            Asset quad = Asset("models/quad.obj", {"models/quad_Color.png"}, {"shaders/vertexShader.vert", "shaders/fragmentShader.frag"}, &renderEngine.settings, {0, 0, 0}, {glm::radians(90.0), 0, 0}, {100, 100, 0});
            Asset cube = Asset("models/cube.obj", {"models/cube.png"}, {"shaders/vertexShader.vert", "shaders/fragmentShader.frag"}, &renderEngine.settings, {0, 0, 0});
            Asset statue = Asset("models/ancientStatue.obj", {"models/ancientStatue.png"}, {"shaders/vertexShader.vert", "shaders/fragmentShader.frag"}, &renderEngine.settings, {7, 2, 0});
            renderEngine.uploadAsset(&cube, true);
            renderEngine.uploadAsset(&quad, true);
            renderEngine.uploadAsset(&vikingRoom, true);
            renderEngine.uploadAsset(&statue, true);
            double lastTab{0};
            double lastF2{0};
            double lastCursorPosX{0};
            double lastCursorPosY{0};
            bool captureInput{};
            while (renderEngine.update()) {
                //Process inputs
                glfwPollEvents();
                float velocity = renderEngine.frameTime * renderEngine.camera.movementSpeed;
                if (glfwGetKey(renderEngine.window, GLFW_KEY_F1)) {
                    for (Asset *asset : renderEngine.assets) { asset->reloadAsset(); }
                    renderEngine.updateSettings(false);
                } if (glfwGetKey(renderEngine.window, GLFW_KEY_F2) & (glfwGetTime() - lastF2 > .2)) {
                    renderEngine.settings.fullscreen = !renderEngine.settings.fullscreen;
                    renderEngine.updateSettings(true);
                    lastF2 = glfwGetTime();
                } if (glfwGetKey(renderEngine.window, GLFW_KEY_1)) {
                    renderEngine.settings.msaaSamples = VK_SAMPLE_COUNT_1_BIT;
                    renderEngine.updateSettings(true);
                } if (glfwGetKey(renderEngine.window, GLFW_KEY_8)) {
                    renderEngine.settings.msaaSamples = VK_SAMPLE_COUNT_8_BIT;
                    renderEngine.updateSettings(true);
                } if (glfwGetKey(renderEngine.window, GLFW_KEY_LEFT_CONTROL) & captureInput) { velocity *= 6; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_W) & captureInput) { renderEngine.camera.position += renderEngine.camera.front * velocity; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_A) & captureInput) { renderEngine.camera.position -= renderEngine.camera.right * velocity; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_S) & captureInput) { renderEngine.camera.position -= renderEngine.camera.front * velocity; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_D) & captureInput) { renderEngine.camera.position += renderEngine.camera.right * velocity; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_LEFT_SHIFT) & captureInput) { renderEngine.camera.position -= renderEngine.camera.up * velocity; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_SPACE) & captureInput) { renderEngine.camera.position += renderEngine.camera.up * velocity; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_TAB) & (glfwGetTime() - lastTab > .2)) {
                    int mode{glfwGetInputMode(renderEngine.window, GLFW_CURSOR)};
                    if (mode == GLFW_CURSOR_DISABLED) {
                        glfwGetCursorPos(renderEngine.window, &lastCursorPosX, &lastCursorPosY);
                        glfwSetInputMode(renderEngine.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                        glfwSetCursorPosCallback(renderEngine.window, nullptr);
                    } else if (mode == GLFW_CURSOR_NORMAL) {
                        if (glfwGetWindowAttrib(renderEngine.window, GLFW_HOVERED)) {
                            glfwSetInputMode(renderEngine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                            glfwSetCursorPos(renderEngine.window, 0, 0);
                            glfwSetCursorPosCallback(renderEngine.window, cursorCallback);
                            glfwSetCursorPos(renderEngine.window, lastCursorPosX, lastCursorPosY);
                        } else {
                            glfwSetCursorPos(renderEngine.window, 0, 0);
                            glfwSetInputMode(renderEngine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                            glfwSetCursorPosCallback(renderEngine.window, cursorCallback);
                        }
                    }
                    lastTab = glfwGetTime();
                    captureInput = !captureInput;
                } if (glfwGetKey(renderEngine.window, GLFW_KEY_ESCAPE)) { glfwSetWindowShouldClose(renderEngine.window, 1); }
                //move assets
                cube.position = {10 * cos(3 * glfwGetTime()), 10 * sin(3 * glfwGetTime()), 1};
            }
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
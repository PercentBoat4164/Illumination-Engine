#include <iostream>
#include "Settings.hpp"
#include "AssetLinking.hpp"
#include "Asset.hpp"
#include "VulkanRenderEngine.hpp"
#include "OpenGLRenderEngine.hpp"


void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    auto RenderEngine = static_cast<VulkanRenderEngine *>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {glfwSetWindowShouldClose(window, 1);}
    else if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        RenderEngine->settings.msaaSamples = VK_SAMPLE_COUNT_1_BIT;
        RenderEngine->updateSettings(RenderEngine->settings, true);
    } else if (key == GLFW_KEY_8 && action == GLFW_PRESS) {
        RenderEngine->settings.msaaSamples = VK_SAMPLE_COUNT_8_BIT;
        RenderEngine->updateSettings(RenderEngine->settings, true);
    }
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
//            glfwSetKeyCallback(renderEngine.window, keyCallback);
            Asset vikingRoom = Asset("models/vikingRoom.obj", {"models/vikingRoom.png"}, {"shaders/vertexShader.vert", "shaders/fragmentShader.frag"}, &renderEngine.settings, {0, 0, 0}, {0, 0, 0}, {1, 1, 1});
            renderEngine.uploadAsset(&vikingRoom);
//            Asset ancientStatue = Asset("models/ancientStatue.obj", {"models/ancientStatue.png"}, {"shaders/vertexShader.vert", "shaders/fragmentShader.frag"}, &renderEngine.settings);
//            renderEngine.uploadAsset(&ancientStatue);
            while (renderEngine.update()) {
                glfwPollEvents();
                static auto startTime = std::chrono::high_resolution_clock::now();
                auto currentTime = std::chrono::high_resolution_clock::now();
                float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
                vikingRoom.rotation = {sin(time), sin(time), sin(time)};
                vikingRoom.position = {sin(time), sin(time), sin(time)};
                vikingRoom.scale = {sin(time), sin(time), sin(time)};
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
            glfwSetKeyCallback(renderEngine.window, keyCallback);
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
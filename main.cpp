#include <iostream>
#include "Settings.hpp"
#include "AssetLinking.hpp"
#include "Asset.hpp"
#include "VulkanRenderEngine.hpp"
#include "OpenGLRenderEngine.hpp"


void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    auto vulkanRenderEngine = static_cast<VulkanRenderEngine *>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        vulkanRenderEngine->settings.resolution = vulkanRenderEngine->settings.defaultMonitorResolution;
        vulkanRenderEngine->settings.fullscreen = true;
        glfwSetWindowShouldClose(window, 1);
    } else if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        vulkanRenderEngine->settings.msaaSamples = VK_SAMPLE_COUNT_1_BIT;
        vulkanRenderEngine->updateSettings(vulkanRenderEngine->settings, true);
    } else if (key == GLFW_KEY_8 && action == GLFW_PRESS) {
        vulkanRenderEngine->settings.msaaSamples = VK_SAMPLE_COUNT_8_BIT;
        vulkanRenderEngine->updateSettings(vulkanRenderEngine->settings, true);
    } else if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        for (Asset *asset : vulkanRenderEngine->assets) { asset->reloadAsset(); }
        vulkanRenderEngine->updateSettings(vulkanRenderEngine->settings, false);
    } else if (key == GLFW_KEY_F2 && action == GLFW_PRESS) {
        Settings newSettings{};
        newSettings = vulkanRenderEngine->settings;
        newSettings.fullscreen = !vulkanRenderEngine->settings.fullscreen;
        vulkanRenderEngine->updateSettings(newSettings, true);
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
            glfwSetKeyCallback(renderEngine.window, keyCallback);
            Asset vikingRoom = Asset("models/vikingRoom.obj", {"models/vikingRoom.png"}, {"shaders/vertexShader.vert", "shaders/fragmentShader.frag"}, &renderEngine.settings);
            Asset ancientStatue = Asset("models/ancientStatue.obj", {"models/ancientStatue.png"}, {"shaders/vertexShader.vert", "shaders/fragmentShader.frag"}, &renderEngine.settings);
            renderEngine.uploadAsset(&ancientStatue);
            int frameNumber{};
            while (renderEngine.update()) {
                frameNumber++;
                glfwPollEvents();
                static auto startTime = std::chrono::high_resolution_clock::now();
                auto currentTime = std::chrono::high_resolution_clock::now();
                float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
                renderEngine.camera.position = {1, 1, 1};
                renderEngine.camera.subjectPosition = {vikingRoom.position};
                if (frameNumber == 1000) { renderEngine.uploadAsset(&vikingRoom); }
            }
            vikingRoom.destroy();
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
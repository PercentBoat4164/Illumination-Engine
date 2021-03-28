#include <iostream>
#include "Settings.hpp"
#include "AssetLinking.hpp"
#include "Asset.hpp"
#include "VulkanRenderEngine.hpp"
#include "OpenGLRenderEngine.hpp"
#include "Physics.hpp"

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

            //physics init
            SphereBody sphere{0,0,0,1,1};
            float g = 0.001;
            //rendering init
            VulkanRenderEngine renderEngine = VulkanRenderEngine();
            glfwSetKeyCallback(renderEngine.window, keyCallback);
            Asset ancientStatue = Asset("models\\ancientStatue.obj", {"models\\ancientStatue.png"}, {"shaders\\vertexShader.vert", "shaders\\fragmentShader.frag"}, &renderEngine.settings, {0, 0, 0}, {0, 0, 0}, {0, 0, 0});
            renderEngine.uploadAsset(&ancientStatue);
            auto currentTime = std::chrono::high_resolution_clock::now();
            while (renderEngine.update()) {

                sphere.applyImpulse(0, 0, -g * sphere.m);
                if(sphere.pos.z < -2) {
                    sphere.applyImpulse(0,0, sphere.v.z * -1.91f);
                }
                sphere.step();
                glfwPollEvents();
                static auto startTime = std::chrono::high_resolution_clock::now();
                auto lastTime = currentTime;
                currentTime = std::chrono::high_resolution_clock::now();
                std::cout << "\nTime per frame" << (currentTime - lastTime);
                float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
                ancientStatue.rotation = {0, 0, 0};
                ancientStatue.position = {sphere.pos};
                ancientStatue.scale = {1, 1, 1};

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
#include "RenderEngine.hpp"
#include "Settings.hpp"

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    auto RenderEngine = static_cast<VulkanRenderEngine *>(glfwGetWindowUserPointer(window));
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {glfwSetWindowShouldClose(window, 1);}
    if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
        RenderEngine->settings.msaaSamples = VK_SAMPLE_COUNT_1_BIT;
        RenderEngine->updateSettings(RenderEngine->settings, true);
    }
    if (key == GLFW_KEY_8 && action == GLFW_PRESS) {
        RenderEngine->settings.msaaSamples = VK_SAMPLE_COUNT_8_BIT;
        RenderEngine->updateSettings(RenderEngine->settings, true);
    }
}

int main() {
    Settings settings{};
    std::cout << "'v': Run Vulkan render engine\n'o': Run OpenGL render engine\n'p': Run Physics\n";
    char input = 'v';
    //std::cin >> input;
    if (input == 'v') {
        try {
            settings.validationLayers = {"VK_LAYER_KHRONOS_validation"};
            VulkanRenderEngine RenderEngine(settings);
            glfwSetKeyCallback(RenderEngine.window, keyCallback);
            settings.findMaxSettings(RenderEngine.physicalDevice);
            settings.msaaSamples = VK_SAMPLE_COUNT_8_BIT;
            settings.resolution = {1280, 720};
            settings.fullscreen = false;
            RenderEngine.start();
            RenderEngineLink renderEngineLink = RenderEngine.createEngineLink();
            //Asset viking_room = Asset().uploadAsset(renderEngineLink);//.load("models/viking_room", ".obj", ".png");
            //RenderEngine.updateSettings(settings, true);
            while (RenderEngine.update() != 1) {
                glfwPollEvents();
            }
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    else if (input == 'o') {
        try {
            OpenGLRenderEngine RenderEngine(settings);
            glfwSetKeyCallback(RenderEngine.window, keyCallback);
            while (RenderEngine.update() != 1) {
                glfwPollEvents();
            }
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
}
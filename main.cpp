#include "RenderEngine.h"

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {glfwSetWindowShouldClose(window, 1);}
}

int main() {
    Settings settings{};
    //settings.validationLayers = {"VK_LAYER_KHRONOS_validation"};
    std::cout << "Use Vulkan? Y/n:\n";
    std::basic_string<char> input;
    //std::cin >> input;
    input = static_cast<std::string>(input);
    if (input == "y") {
        try {
            VulkanRenderEngine RenderEngine(settings);
            settings.findMaxSettings(RenderEngine.physicalDevice);
            settings.fullscreen = false;
            RenderEngine.updateSettings(settings, true);
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
    else {
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
#include "vulkanRenderEngine.hpp"

#include <iostream>

/*
 * Any code for testing Vulkan goes here.
 */
int main(int argc, char **argv) {
    if (!glfwVulkanSupported() & (*(argv[1]) != 'f')) { throw std::runtime_error("Vulkan is not supported on this device."); } else { std::cerr << "Attempting to run Vulkan on unsupported device!" << std::endl; }
    VulkanRenderEngine renderEngine{};
    VulkanRenderable cube{&renderEngine.renderEngineLink, "res/Models/Cube/cube.obj"};
    cube.position = {0, -3, 0};
    renderEngine.loadRenderable(&cube);
    while (renderEngine.update()) {
        glfwPollEvents();
    }
    renderEngine.destroy();
}
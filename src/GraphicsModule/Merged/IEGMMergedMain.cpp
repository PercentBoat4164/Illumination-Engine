#include "IERenderEngine.hpp"

#include <iostream>

/*
 * Any code for testing the merged graphics engine goes here.
 */
int main(int argc, char **argv) {
    std::string selection;
    std::string input;
    bool force{false};
    if (argc > 1) {
        if (*argv[1] == 'v') { input = IE_RENDER_ENGINE_API_NAME_VULKAN; }
        if (*argv[1] == 'o') { input = IE_RENDER_ENGINE_API_NAME_OPENGL; }
        force = *(argv[1] + 1) == 'f';
    } if (input.empty()) {
        std::cout << "'v': Run Vulkan render engine\n'o': Run OpenGL render engine\n";
        std::cin >> selection;
        if (selection[0] == 'v') { input = IE_RENDER_ENGINE_API_NAME_VULKAN; }
        if (selection[0] == 'o') { input = IE_RENDER_ENGINE_API_NAME_OPENGL; }
    } if (input == IE_RENDER_ENGINE_API_NAME_VULKAN & !glfwVulkanSupported() & !force) {
        input = IE_RENDER_ENGINE_API_NAME_OPENGL;
        std::cout << "Forcing OpenGL because Vulkan is not supported" << std::endl;
    }
    Log log{};
    log.create("IE");
    log.addModule("IEGMMergedMain");
    log.log("Choosing " + input + " API", log4cplus::INFO_LOG_LEVEL, "IEGMMergedMain");
    IERenderEngine renderEngine{input, &log};
    renderEngine.create();
    IERenderable cube{""};
    while (renderEngine.update()) {
        glfwPollEvents();
    }
}
#include "IeRenderEngine.hpp"

#include <iostream>

int main(int argc, char **argv) {
    std::string selection;
    std::string input;
    bool force{false};
    if (argc > 1) {
        if (*argv[1] == 'v') { input = "Vulkan"; }
        if (*argv[1] == 'o') { input = "OpenGL"; }
        force = *(argv[1] + 1) == 'f';
    } if (input.empty()) {
        std::cout << "'v': Run Vulkan render engine\n'o': Run OpenGL render engine\n";
        std::cin >> selection;
        if (selection[0] == 'v') { input = "Vulkan"; }
        if (selection[0] == 'o') { input = "OpenGL"; }
    } if (input == "Vulkan" & !glfwVulkanSupported() & !force) {
        input = "OpenGL";
        std::cout << "Forcing OpenGL because Vulkan is not supported" << std::endl;
    }
    Log log{};
    log.create("IE");
    log.addModule("IEGMMergedMain");
    log.log("Choosing " + input + " API", log4cplus::INFO_LOG_LEVEL, "IEGMMergedMain");
    IeRenderEngine renderEngine{input, &log};
    renderEngine.create();
}
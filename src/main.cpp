#include <iostream>

#include "GraphicsModule/Merged/RenderEngine.hpp"

int main(int argc, char **argv) {
    std::string selection;
    std::string input;
    if (argc > 1) {
        if (*argv[1] == 'v') { input = "Vulkan"; }
        if (*argv[1] == 'o') { input = "OpenGL"; }
    } else {
        std::cout << "'v': Run Vulkan render engine\n'o': Run OpenGL render engine\n";
        std::cin >> selection;
        if (selection[0] == 'v') { input = "Vulkan"; }
        if (selection[0] == 'o') { input = "OpenGL"; }
    }
    Log log{};
    log.create("Illumination Engine");
    RenderEngine renderEngine{input, &log};
    renderEngine.create();
}
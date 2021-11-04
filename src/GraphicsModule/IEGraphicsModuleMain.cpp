#include "Vulkan/vulkanRenderEngine.hpp"
#include "OpenGL/openglRenderEngine.hpp"
#include "Merged/IeRenderEngine.hpp"
#include "LogModule/Log.hpp"

/*
 * Any code for testing the graphics engine goes here.
 */
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
    VulkanRenderEngine vRenderEngine{};
    OpenGLRenderEngine oRenderEngine{};
    Log log{};
    IeRenderEngine mRenderEngine{input, &log};
    mRenderEngine.create();
    VulkanRenderable vCube{&vRenderEngine.renderEngineLink, "res/Models/Cube/Cube.obj"};
    OpenGLRenderable oCube{&oRenderEngine.renderEngineLink, "res/Models/Cube/Cube.obj"};
    vRenderEngine.loadRenderable(&vCube);
    oRenderEngine.loadRenderable(&oCube);
}
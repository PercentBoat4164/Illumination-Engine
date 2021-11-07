#ifndef VMA_INCLUDED
#define VMA_INCLUDED
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#endif

#include "Vulkan/vulkanRenderEngine.hpp"
#include "OpenGL/openglRenderEngine.hpp"
#include "Merged/IeRenderEngine.hpp"
#include "LogModule/Log.hpp"

/*
 * Any code for testing the entire graphics module goes here.
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
    OpenGLRenderEngine oRenderEngine{};
    OpenGLRenderable oCube{&oRenderEngine.renderEngineLink, "res/Models/Cube/cube.obj"};
    oRenderEngine.loadRenderable(&oCube);
    oCube.position = {0, -3, 0};
    while (oRenderEngine.update()) {
        glfwPollEvents();
    }
    oRenderEngine.destroy();
    VulkanRenderEngine vRenderEngine{};
    VulkanRenderable vCube{&vRenderEngine.renderEngineLink, "res/Models/Cube/cube.obj"};
    vRenderEngine.loadRenderable(&vCube);
    vCube.position = {0, -3, 0};
    while (vRenderEngine.update()) {
        glfwPollEvents();
    }
    vRenderEngine.destroy();
    Log log{};
    IeRenderEngine mRenderEngine{input, &log};
    mRenderEngine.create();
    while (oRenderEngine.update()) {
        glfwPollEvents();
    }
    mRenderEngine.destroy();
}
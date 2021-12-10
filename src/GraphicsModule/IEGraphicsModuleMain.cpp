#ifndef VMA_INCLUDED
#define VMA_INCLUDED
#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>
#endif

#include "Vulkan/vulkanRenderEngine.hpp"
#include "OpenGL/openglRenderEngine.hpp"

/*
 * Any code for testing the entire graphics module goes here.
 */
int main(int argc, char **argv) {
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
}
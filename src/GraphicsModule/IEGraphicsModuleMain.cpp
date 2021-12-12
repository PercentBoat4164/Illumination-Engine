#ifdef ILLUMINATION_ENGINE_VULKAN
#include "Vulkan/vulkanRenderEngine.hpp"
#endif
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
    #ifdef ILLUMINATION_ENGINE_VULKAN
    VulkanRenderEngine vRenderEngine{};
    VulkanRenderable vCube{&vRenderEngine.renderEngineLink, "res/Models/Cube/cube.obj"};
    vRenderEngine.loadRenderable(&vCube);
    vCube.position = {0, -3, 0};
    while (vRenderEngine.update()) {
        glfwPollEvents();
    }
    vRenderEngine.destroy();
    #endif
}
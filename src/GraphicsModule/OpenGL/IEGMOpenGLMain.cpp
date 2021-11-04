#include "openglRenderEngine.hpp"

#include <iostream>

/*
 * Any code for testing OpenGL goes here.
 */
int main(int argc, char **argv) {
    OpenGLRenderEngine renderEngine{};
    OpenGLRenderable cube{&renderEngine.renderEngineLink, "res/Models/Cube/cube.obj"};
    renderEngine.loadRenderable(&cube);
    cube.position = {0, -3, 0};
    while (renderEngine.update()) {
        glfwPollEvents();
    }
    renderEngine.destroy();
}
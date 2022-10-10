#include "Core/Core.hpp"
#include "RenderEngine.hpp"

int main() {
    IE::Graphics::RenderEngine renderEngine{};
    while (!glfwWindowShouldClose(renderEngine.getWindow())) glfwPollEvents();
}

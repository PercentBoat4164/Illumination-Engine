#include "Core/Core.hpp"
#include "RenderEngine.hpp"

int main() {
    auto renderEngine{IE::Graphics::RenderEngine::create()};
    while (!glfwWindowShouldClose(renderEngine->getWindow())) glfwPollEvents();
    renderEngine->destroy();
}

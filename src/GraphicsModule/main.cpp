#include "RenderEngine.hpp"

int main() {
    auto renderEngine{IE::Graphics::RenderEngine::create()};
    while (glfwWindowShouldClose(renderEngine->getWindow()) == 0) glfwPollEvents();
}

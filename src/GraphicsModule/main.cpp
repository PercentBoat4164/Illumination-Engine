
#include "Core/Core.hpp"
#include "RenderEngine.hpp"

int main() {
    auto core{std::make_shared<IE::Core::Core>()};
    auto renderEngine{IE::Graphics::RenderEngine::create(core)};
    while (!glfwWindowShouldClose(renderEngine->getWindow())) glfwPollEvents();
}
#include "Core/Core.hpp"
#include "GraphicsModule/Image/ImageVulkan.hpp"
#include "RenderEngine.hpp"

int main() {
    auto *renderEngine = IE::Core::Core::createEngine<IE::Graphics::RenderEngine>("renderEngine");
    //    while (glfwWindowShouldClose(renderEngine->getWindow()) == 0) glfwPollEvents();
}
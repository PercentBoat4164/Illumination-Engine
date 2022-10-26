#include "RenderEngine.hpp"

int main() {
    auto *renderEngine{IE::Core::Core::createEngine<IE::Graphics::RenderEngine>("render engine")};
    while (glfwWindowShouldClose(renderEngine->getWindow()) == 0) glfwPollEvents();
}

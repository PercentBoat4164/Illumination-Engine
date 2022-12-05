#include "Core/AssetModule/Asset.hpp"
#include "Core/Core.hpp"
#include "GraphicsModule/Image/ImageVulkan.hpp"
#include "RenderEngine.hpp"

int main(int argc, char **argv) {
    if (argc >= 1) {
        std::string programLocation  = std::string(argv[0]);
        std::string resourceLocation = programLocation.substr(0, programLocation.find_last_of('/'));
        IE::Core::Core::getInst(resourceLocation);
    }

    auto           *renderEngine = IE::Core::Core::createEngine<IE::Graphics::RenderEngine>("renderEngine");
    IE::Core::Asset asset(IE::Core::Core::getFileSystem()->getFile("res/assets/AncientStatue"));
    asset.addAspect(renderEngine->createAspect(
      "AncientStatue",
      IE::Core::Core::getFileSystem()->getFile("res/assets/AncientStatue/models/ancientStatue.glb")
    ));

    while (glfwWindowShouldClose(renderEngine->getWindow()) == 0) {
        renderEngine->update();
        glfwPollEvents();
        renderEngine->finish();
    }
}
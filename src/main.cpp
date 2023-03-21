#include "Core/AssetModule/Asset.hpp"
#include "Core/Core.hpp"
#include "RenderEngine.hpp"

int main(int argc, char **argv) {
    if (argc >= 1) {
        std::string programLocation  = std::string(argv[0]);
        std::string resourceLocation = programLocation.substr(0, programLocation.find_last_of('/'));
        IE::Core::Core::getInst(resourceLocation);
    }

    auto renderEngineCreator = IE::Core::Core::getThreadPool()->submit(
      IE::Core::Core::createEngine<IE::Graphics::RenderEngine>,
      "render engine"
    );
    renderEngineCreator->wait();
    std::shared_ptr<IE::Graphics::RenderEngine> renderEngine = renderEngineCreator->value();

    IE::Core::Asset asset(IE::Core::Core::getFileSystem()->getFile("res/assets/AncientStatue"));
    asset.addAspect(renderEngine->createAspect(
      "AncientStatue",
      IE::Core::Core::getFileSystem()->getFile("res/assets/AncientStatue/models/ancientStatue.glb")
    ));

    IE::Core::Core::getLogger()->log("Initialization finished successfully. Starting main loop.");

    while (glfwWindowShouldClose(renderEngine->getWindow()) == 0) {
        auto job = IE::Core::Core::getThreadPool()->submit(renderEngine->update());
        glfwPollEvents();
        job->wait();
    }
}
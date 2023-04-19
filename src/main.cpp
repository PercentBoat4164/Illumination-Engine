#include "CommandBuffer/CommandPool.hpp"
#include "Core/AssetModule/Asset.hpp"
#include "Core/Core.hpp"
#include "Core/FileSystemModule/FileSystem.hpp"
#include "RenderEngine.hpp"

IE::Core::Threading::CoroutineTask<void> illuminationEngine() {
    auto renderEngineCreator = IE::Core::Core::getThreadPool().submit(
      IE::Core::Core::createEngine<IE::Graphics::RenderEngine>,
      "render engine"
    );

    co_await IE::Core::Core::getThreadPool().resumeAfter(renderEngineCreator);

    std::shared_ptr<IE::Graphics::RenderEngine> renderEngine = renderEngineCreator->value();

    IE::Core::Asset asset(IE::Core::Core::getFileSystem().getFile("res/assets/AncientStatue"));
    asset.addAspect(renderEngine->createAspect(
      "AncientStatue",
      IE::Core::Core::getFileSystem().getFile("res/assets/AncientStatue/models/ancientStatue.glb")
    ));

    IE::Core::Core::getLogger().log("Initialization finished successfully. Starting main loop.");

    //    while (glfwWindowShouldClose(renderEngine->getWindow()) == 0) {
    //        auto job = IE::Core::Core::getThreadPool()->submit(renderEngine->update());
    //        glfwPollEvents();
    //        job->wait();
    //    }

    IE::Core::Core::getThreadPool().shutdown();
}

int main(int argc, char **argv) {
    if (argc >= 1) IE::Core::Core::getInst(std::filesystem::path(argv[0]).parent_path().string());

    auto job = IE::Core::Core::getThreadPool().submit(illuminationEngine);

    IE::Core::Core::getThreadPool().startMainThreadLoop();

    job->wait();
}
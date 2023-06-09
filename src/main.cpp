#include "CommandBuffer/CommandPool.hpp"
#include "Core/AssetModule/Asset.hpp"
#include "Core/Core.hpp"
#include "Core/FileSystemModule/FileSystem.hpp"
#include "Core/ThreadingModule/EnsureThread.hpp"
#include "Core/ThreadingModule/Task.hpp"
#include "RenderEngine.hpp"

#include <iostream>

IE::Core::Threading::Task<void> illuminationEngine() {
    std::shared_ptr<IE::Graphics::RenderEngine> renderEngine =
      IE::Core::Core::createEngine<IE::Graphics::RenderEngine>("render engine");

    //    auto renderEngineCreator = IE::Core::Core::getThreadPool().prepareAndSubmit(renderEngine->create());
    //    co_await IE::Core::Core::getThreadPool().resumeAfter(renderEngineCreator);

    IE::Core::Core::getThreadPool().executeInPlace(renderEngine->create());


    IE::Core::Asset asset(IE::Core::Core::getFileSystem().getFile("res/assets/AncientStatue"));
    asset.addAspect(renderEngine->createAspect(
      "AncientStatue",
      IE::Core::Core::getFileSystem().getFile("res/assets/AncientStatue/models/ancientStatue.glb")
    ));

    IE::Core::Core::getLogger().log("Initialization finished successfully. Starting main loop.");

    //    while (glfwWindowShouldClose(renderEngine->getWindow()) == 0) {
    //        auto job = IE::Core::Core::getThreadPool()->prepareAndSubmit(renderEngine->update());
    //        glfwPollEvents();
    //        job->wait();
    //    }

    IE::Core::Core::getThreadPool().shutdown();

    co_return;
}

int main(int argc, char **argv) {
    if (argc >= 1) IE::Core::Core::getInst(std::filesystem::path(argv[0]).parent_path().string());

    auto job = IE::Core::Core::getThreadPool().submit(illuminationEngine());

    IE::Core::Core::getThreadPool().startMainThreadLoop();
}
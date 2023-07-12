#include "Core/Core.hpp"
#include "RenderEngine.hpp"

#include <iostream>

IE::Core::Threading::Task<void> illuminationEngine() {
    std::shared_ptr<IE::Graphics::RenderEngine> renderEngine =
      IE::Core::createEngine<IE::Graphics::RenderEngine>("render engine");

    IE::Core::getThreadPool().executeInPlace(renderEngine->create());

    IE::Core::Asset asset(IE::Core::getFileSystem().getFile("res/assets/AncientStatue"));
    asset.addAspect(renderEngine->createAspect(
      "AncientStatue",
      IE::Core::getFileSystem().getFile("res/assets/AncientStatue/models/ancientStatue.glb")
    ));

    IE::Core::getLogger().log("Initialization finished successfully. Starting main loop.");

    //    while (glfwWindowShouldClose(renderEngine->getWindow()) == 0) {
    //        auto job = IE::Core::Core::getThreadPool()->prepareAndSubmit(renderEngine->update());
    //        glfwPollEvents();
    //        job->wait();
    //    }

    IE::Core::getThreadPool().shutdown();

    co_return;
}

int main(int argc, char **argv) {
        auto job = IE::Core::getThreadPool().submit(illuminationEngine());

        IE::Core::getThreadPool().startMainThreadLoop();
}
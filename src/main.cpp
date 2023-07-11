#include "Core/Core.hpp"
#include "GraphicsModule/IERenderEngine.hpp"

IE::Core::Threading::Task<void> illuminationEngine() {
    IESettings                      settings{};

    auto r = IE::Core::getThreadPool().submit(IE::Core::createEngine<IERenderEngine>("render engine", settings));
    co_await IE::Core::getThreadPool().resumeAfter(r);

    std::shared_ptr<IERenderEngine> renderEngine = r->value();

    IE::Core::Asset asset(IE::Core::getFileSystem().getFile("res/assets/AncientStatue"));
    asset.addAspect(renderEngine->createRenderable(
      "AncientStatue",
      IE::Core::getFileSystem().getFile("res/assets/AncientStatue/models/ancientStatue.glb")
    ));

    IE::Core::getLogger().log("Initialization finished successfully. Starting main loop.");

    while (renderEngine->update()) glfwPollEvents();

    IE::Core::getThreadPool().shutdown();

    co_return;
}

int main() {
    IE::Core::getThreadPool().submit(illuminationEngine());

    IE::Core::getThreadPool().startMainThreadLoop();
}
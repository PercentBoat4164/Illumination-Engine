#include "Core/Core.hpp"
#include "GraphicsModule/IERenderEngine.hpp"

IE::Core::Threading::Task<void> illuminationEngine() {
    IESettings                      settings{};
    std::shared_ptr<IERenderEngine> renderEngine =
      IE::Core::createEngine<IERenderEngine>("render engine", settings);

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

int main(int argc, char **argv) {
    if (argc >= 1) IE::Core::init(std::filesystem::path(argv[0]).parent_path().string());

    auto job = IE::Core::getThreadPool().submit(illuminationEngine());

    IE::Core::getThreadPool().startMainThreadLoop();
}
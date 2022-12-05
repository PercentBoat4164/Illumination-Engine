#include "Core/Core.hpp"
#include "IERenderEngine.hpp"
#include "InputModule/InputEngine.hpp"
#include "InputModule/Keyboard.hpp"
#include "ScriptingModule/Script.hpp"

#include <iostream>

int main(int argc, char **argv) {
    if (argc >= 1) {
        std::string programLocation  = std::string(argv[0]);
        std::string resourceLocation = programLocation.substr(0, programLocation.find_last_of('/'));
        IE::Core::Core::getInst(resourceLocation);
    }

    IE::Core::ThreadPool threadPool{};

    auto script = IE::Script::Script::create(
      IE::Core::Core::getFileSystem()->getFile("res/assets/AncientStatue/scripts/rotate.py")
    );
    std::future<void> script_load{IE::Core::Core::getThreadPool()->submit([&] {
        script->compile();
        script->load();
        script->initialize();
    })};

    IESettings               settings     = IESettings();
    IERenderEngine          *renderEngine = IE::Core::Core::createEngine<IERenderEngine>("render engine");
    std::shared_ptr<IEAsset> pythonRunner = std::make_shared<IEAsset>();
    pythonRunner->filename                = "res/assets/AncientStatue/models/ancientStatue.fbx";
    pythonRunner->position                = {0, 3, 0};
    pythonRunner->addAspect(new IERenderable);
    pythonRunner->addAspect(script.get());
    renderEngine->addAsset(pythonRunner);

    // RenderEngine must be allocated on the heap.

    renderEngine->settings->logger.log(
      "Beginning main loop.",
      IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_INFO
    );

    glfwSetTime(0.0);
    script_load.wait();
    while (renderEngine->update()) {
        std::future<void> script_execute{IE::Core::Core::getThreadPool()->submit([&] { script->update(); })};
        glfwPollEvents();
        script_execute.wait();
    }
}
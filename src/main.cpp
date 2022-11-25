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

    auto script = IE::Script::Script::create(
      IE::Core::Core::getFileSystem()->getFile("res/assets/AncientStatue/scripts/rotate.py")
    );
    script->compile();
    script->initialize();
    IESettings      settings     = IESettings();
    IERenderEngine *renderEngine = IE::Core::Core::createEngine<IERenderEngine>("render engine");

    // RenderEngine must be allocated on the heap.
    IE::Core::ThreadPool threadPool{};

    renderEngine->settings->logger.log(
      "Beginning main loop.",
      IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_INFO
    );

    glfwSetTime(0.0);
    while (renderEngine->update()) {
        glfwPollEvents();
        script->update();
    }
}
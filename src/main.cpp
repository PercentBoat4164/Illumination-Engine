#include "Core/Core.hpp"
#include "IERenderEngine.hpp"
#include "InputModule/InputEngine.hpp"
#include "InputModule/Keyboard.hpp"
#include "ScriptingModule/Script.hpp"

#include <iostream>

int main(int argc, char **argv) {
    if (argc >= 1) {
        auto programLocation  = std::filesystem::path(argv[0]);
        std::string const resourceLocation = programLocation.parent_path().string();
        IE::Core::Core::getInst(resourceLocation);
    }

    const IESettings settings     = IESettings();
    auto            *renderEngine = IE::Core::Core::createEngine<IERenderEngine>("render engine");

    IE::Input::InputEngine inputEngine{renderEngine->window};
    std::shared_ptr<IE::Input::Keyboard> keyboard = inputEngine.getAspect("keyboard");
    keyboard->editActions(
      GLFW_KEY_W,
      [&](GLFWwindow *) {
          renderEngine->camera.position +=
            renderEngine->camera.front * renderEngine->frameTime * renderEngine->camera.speed;
      }
    );
    keyboard->editActions(
      GLFW_KEY_A,
      [&](GLFWwindow *) {
          renderEngine->camera.position -=
            renderEngine->camera.right * renderEngine->frameTime * renderEngine->camera.speed;
      }
    );
    keyboard->editActions(
      GLFW_KEY_S,
      [&](GLFWwindow *) {
          renderEngine->camera.position -=
            renderEngine->camera.front * renderEngine->frameTime * renderEngine->camera.speed;
      }
    );
    keyboard->editActions(
      GLFW_KEY_D,
      [&](GLFWwindow *) {
          renderEngine->camera.position +=
            renderEngine->camera.right * renderEngine->frameTime * renderEngine->camera.speed;
      }
    );
    keyboard->editActions(
      GLFW_KEY_SPACE,
      [&](GLFWwindow *) {
          renderEngine->camera.position +=
            renderEngine->camera.up * renderEngine->frameTime * renderEngine->camera.speed;
      }
    );
    keyboard->editActions(
      GLFW_KEY_LEFT_SHIFT,
      [&](GLFWwindow *) {
          renderEngine->camera.position -=
            renderEngine->camera.up * renderEngine->frameTime * renderEngine->camera.speed;
      }
    );
    keyboard->editActions(
      {GLFW_KEY_LEFT_CONTROL, GLFW_PRESS},
      [&](GLFWwindow *) { renderEngine->camera.speed *= 6.0; }
    );
    keyboard->editActions(
      {GLFW_KEY_LEFT_CONTROL, GLFW_RELEASE},
      [&](GLFWwindow *) { renderEngine->camera.speed /= 6.0; }
    );
    keyboard->editActions(
      {GLFW_KEY_F11, GLFW_PRESS},
      [&](GLFWwindow *) { renderEngine->toggleFullscreen(); }
    );
    keyboard->editActions({GLFW_KEY_ESCAPE, GLFW_REPEAT}, [&](GLFWwindow *) {
        glfwSetWindowShouldClose(renderEngine->window, 1);
    });

    std::shared_ptr<IEAsset> fbx(std::make_shared<IEAsset>());
    fbx->filename = "res/assets/AncientStatue/models/ancientStatue.fbx";
    fbx->position = {2, 1, 0};
    fbx->addAspect(new IERenderable{});
    renderEngine->addAsset(fbx);
    std::shared_ptr<IEAsset> obj = std::make_shared<IEAsset>();
    obj->filename                = "res/assets/AncientStatue/models/ancientStatue.obj";
    obj->addAspect(new IERenderable{});
    obj->position = {0, 1, 0};
    renderEngine->addAsset(obj);
    std::shared_ptr<IEAsset> glb = std::make_shared<IEAsset>();
    glb->filename                = "res/assets/AncientStatue/models/ancientStatue.glb";
    glb->addAspect(new IERenderable{});
    glb->position = {-2, 1, 0};
    renderEngine->addAsset(glb);
    std::shared_ptr<IEAsset> floor = std::make_shared<IEAsset>();
    floor->filename                = "res/assets/DeepslateFloor/models/DeepslateFloor.fbx";
    floor->addAspect(new IERenderable{});
    renderEngine->addAsset(floor);
    floor->position = {0, 0, -1};

    renderEngine->camera.position = {0.0F, -2.0F, 1.0F};

    IE::Core::ThreadPool threadPool{};

    auto script = IE::Script::Script::create(
      IE::Core::Core::getFileSystem()->getFile("res/assets/AncientStatue/scripts/rotate.py")
    );
    std::future<void> script_load{IE::Core::Core::getThreadPool()->submit([&] {
        script->compile();
        script->load();
        script->initialize();
    })};

    floor->addAspect(script.get());

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
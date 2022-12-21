#include "Core/Core.hpp"
#include "IERenderEngine.hpp"
#include "InputModule/InputEngine.hpp"
#include "InputModule/Keyboard.hpp"
#include "ScriptingModule/Script.hpp"
#include "ScriptingModule/ScriptEngine.hpp"

#include <iostream>

int main(int argc, char **argv) {
    if (argc >= 1) {
        auto              programLocation  = std::filesystem::path(argv[0]);
        std::string const resourceLocation = programLocation.parent_path().string();
        IE::Core::Core::getInst(resourceLocation);
    }

    const IESettings                settings     = IESettings();
    std::shared_ptr<IERenderEngine> renderEngine = IE::Core::Core::createEngine<IERenderEngine>("render engine");

    std::shared_ptr<IE::Input::InputEngine> inputEngine =
      IE::Core::Core::createEngine<IE::Input::InputEngine>("input engine", renderEngine->window);
    std::shared_ptr<IE::Input::Keyboard> keyboard = inputEngine->getAspect("keyboard");
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

    std::shared_ptr<IE::Core::Asset> fbx{
      std::make_shared<IE::Core::Asset>(IE::Core::Core::getFileSystem()->getFile("res/assets/AncientStatue"))};
    std::shared_ptr<IERenderable> fbxRenderable = renderEngine->createAspect(
      "fbx",
      IE::Core::Core::getFileSystem()->getFile("res/assets/AncientStatue/models/ancientStatue.fbx")
    );
    fbxRenderable->position = {2, 1, 0};
    fbx->addAspect(fbxRenderable);
    renderEngine->addAsset(fbx);


    std::shared_ptr<IE::Core::Asset> obj{
      std::make_shared<IE::Core::Asset>(IE::Core::Core::getFileSystem()->getFile("res/assets/AncientStatue"))};
    std::shared_ptr<IERenderable> objRenderable = renderEngine->createAspect(
      "obj",
      IE::Core::Core::getFileSystem()->getFile("res/assets/AncientStatue/models/ancientStatue.obj")
    );
    objRenderable->position = {0, 1, 0};
    obj->addAspect(objRenderable);
    renderEngine->addAsset(obj);


    std::shared_ptr<IE::Core::Asset> glb{
      std::make_shared<IE::Core::Asset>(IE::Core::Core::getFileSystem()->getFile("res/assets/AncientStatue"))};
    std::shared_ptr<IERenderable> glbRenderable = renderEngine->createAspect(
      "glb",
      IE::Core::Core::getFileSystem()->getFile("res/assets/AncientStatue/models/ancientStatue.glb")
    );
    glbRenderable->position = {-2, 1, 0};
    glb->addAspect(glbRenderable);
    renderEngine->addAsset(glb);


    std::shared_ptr<IE::Core::Asset> floor{
      std::make_shared<IE::Core::Asset>(IE::Core::Core::getFileSystem()->getFile("res/assets/DeepslateFloor"))};
    std::shared_ptr<IERenderable> floorRenderable = renderEngine->createAspect(
      "floor",
      IE::Core::Core::getFileSystem()->getFile("res/assets/DeepslateFloor/models/DeepslateFloor.fbx")
    );
    floorRenderable->position = {0, 0, -1};
    floor->addAspect(floorRenderable);
    renderEngine->addAsset(floor);

    renderEngine->camera.position = {0.0F, -2.0F, 1.0F};

    IE::Core::ThreadPool threadPool{};

    std::shared_ptr<IE::Script::ScriptEngine> scriptEngine =
      IE::Core::Core::createEngine<IE::Script::ScriptEngine>("script engine");

    auto script = scriptEngine->createAspect(
      "rotate",
      IE::Core::Core::getFileSystem()->getFile("res/assets/AncientStatue/scripts/rotate.py")
    );
    std::future<void> script_load{IE::Core::Core::getThreadPool()->submit([&] {
        script->compile();
        script->load();
        script->initialize();
    })};

    floor->addAspect(script);

    // RenderEngine must be allocated on the heap.

    renderEngine->settings->logger.log(
      "Beginning main loop.",
      IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_INFO
    );

    glfwSetTime(0.0);
    script_load.wait();
    while (renderEngine->update()) {
        glfwPollEvents();
        script->update();
        keyboard->handleQueue();
    }
}
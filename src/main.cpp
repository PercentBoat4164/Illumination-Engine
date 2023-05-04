#include "Core/AssetModule/Aspect.hpp"
#include "Core/Core.hpp"
#include "Core/ThreadingModule/CoroutineTask.hpp"
#include "IERenderEngine.hpp"
#include "InputModule/InputEngine.hpp"
#include "InputModule/Keyboard.hpp"

#include <GLFW/glfw3.h>

IE::Core::Threading::CoroutineTask<void> illuminationEngine() {
    IESettings settings     = IESettings();
    auto      *renderEngine = IE::Core::Core::createEngine<IERenderEngine>("render engine", settings);

    IE::Input::InputEngine inputEngine{renderEngine->window};
    IE::Input::Keyboard   *keyboard = inputEngine.getAspect("keyboard");
    keyboard->editActions(GLFW_KEY_W, [&](GLFWwindow *) {
        renderEngine->camera.position +=
          renderEngine->camera.front * renderEngine->frameTime * renderEngine->camera.speed;
    });
    keyboard->editActions(GLFW_KEY_A, [&](GLFWwindow *) {
        renderEngine->camera.position -=
          renderEngine->camera.right * renderEngine->frameTime * renderEngine->camera.speed;
    });
    keyboard->editActions(GLFW_KEY_S, [&](GLFWwindow *) {
        renderEngine->camera.position -=
          renderEngine->camera.front * renderEngine->frameTime * renderEngine->camera.speed;
    });
    keyboard->editActions(GLFW_KEY_D, [&](GLFWwindow *) {
        renderEngine->camera.position +=
          renderEngine->camera.right * renderEngine->frameTime * renderEngine->camera.speed;
    });
    keyboard->editActions(GLFW_KEY_SPACE, [&](GLFWwindow *) {
        renderEngine->camera.position +=
          renderEngine->camera.up * renderEngine->frameTime * renderEngine->camera.speed;
    });
    keyboard->editActions(GLFW_KEY_LEFT_SHIFT, [&](GLFWwindow *) {
        renderEngine->camera.position -=
          renderEngine->camera.up * renderEngine->frameTime * renderEngine->camera.speed;
    });
    keyboard->editActions({GLFW_KEY_LEFT_CONTROL, GLFW_PRESS}, [&](GLFWwindow *) {
        renderEngine->camera.speed *= 6.0;
    });
    keyboard->editActions({GLFW_KEY_LEFT_CONTROL, GLFW_RELEASE}, [&](GLFWwindow *) {
        renderEngine->camera.speed /= 6.0;
    });
    keyboard->editActions({GLFW_KEY_F11, GLFW_PRESS}, [&](GLFWwindow *) {
        renderEngine->queueToggleFullscreen();
    });
    keyboard->editActions({GLFW_KEY_ESCAPE, GLFW_REPEAT}, [&](GLFWwindow *) {
        glfwSetWindowShouldClose(renderEngine->window, 1);
    });

    auto &am       = IE::Core::Core::getAssetManager();
    auto *fbxModel = am.createAspect<IERenderable>(
      "ancientStatue FBX",
      std::filesystem::path("res") / "assets" / "AncientStatue" / "models" / "ancientStatue.fbx"
    );
    am.createAsset("ancientStatues", fbxModel);

    std::shared_ptr<IE::Core::Asset> fbx{std::make_shared<IE::Core::Asset>()};
    fbx->m_filename = "res/assets/AncientStatue/models/ancientStatue.fbx";
    fbx->m_position = {2, 1, 0};
    fbx->addInstance(new IERenderable{});
    renderEngine->addAsset(fbx);
    std::shared_ptr<IE::Core::Asset> obj{std::make_shared<IE::Core::Asset>()};
    obj->m_filename = "res/assets/AncientStatue/models/ancientStatue.obj";
    obj->addInstance(new IERenderable{});
    obj->m_position = {0, 1, 0};
    renderEngine->addAsset(obj);
    std::shared_ptr<IE::Core::Asset> glb{std::make_shared<IE::Core::Asset>()};
    glb->m_filename = "res/assets/AncientStatue/models/ancientStatue.glb";
    glb->addInstance(new IERenderable{});
    glb->m_position = {-2, 1, 0};
    renderEngine->addAsset(glb);
    std::shared_ptr<IE::Core::Asset> floor{std::make_shared<IE::Core::Asset>()};
    floor->m_filename = "res/assets/DeepslateFloor/models/DeepslateFloor.fbx";
    floor->addInstance(new IERenderable{});
    renderEngine->addAsset(floor);
    floor->m_position = {0, 0, -1};

    renderEngine->camera.position = {0.0F, -2.0F, 1.0F};

    settings.logger.log("Beginning main loop.", IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_INFO);

    glfwSetTime(0.0);
    while (renderEngine->update()) {
        glfwPollEvents();
        keyboard->handleQueue();
    }

    IE::Core::Core::getThreadPool().shutdown();

    co_return;
}

int main(int argc, char **argv) {
    if (argc >= 1) IE::Core::Core::getInst(std::filesystem::path(argv[0]).parent_path().string());

    auto main = IE::Core::Core::getThreadPool().submitToMainThread(illuminationEngine);
    IE::Core::Core::getThreadPool().startMainThreadLoop();
    main->wait();
}
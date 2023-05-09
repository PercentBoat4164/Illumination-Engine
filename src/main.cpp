#include "Core/AssetModule/Aspect.hpp"
#include "Core/Core.hpp"
#include "Core/InputModule/InputHandler.hpp"
#include "Core/ThreadingModule/CoroutineTask.hpp"
#include "IERenderEngine.hpp"

#include <GLFW/glfw3.h>

IE::Core::Threading::CoroutineTask<void> illuminationEngine() {
    IESettings settings     = IESettings();
    auto      *renderEngine = IE::Core::Core::createEngine<IERenderEngine>("render engine", settings);

    auto &keyboard = IE::Core::Core::getInputHandler().m_keyboard;
    keyboard.addWindow(renderEngine->window);
    keyboard.editActions(GLFW_KEY_W, [&](GLFWwindow *) {
        renderEngine->camera.position +=
          renderEngine->camera.front * renderEngine->frameTime * renderEngine->camera.speed;
    });
    keyboard.editActions(GLFW_KEY_A, [&](GLFWwindow *) {
        renderEngine->camera.position -=
          renderEngine->camera.right * renderEngine->frameTime * renderEngine->camera.speed;
    });
    keyboard.editActions(GLFW_KEY_S, [&](GLFWwindow *) {
        renderEngine->camera.position -=
          renderEngine->camera.front * renderEngine->frameTime * renderEngine->camera.speed;
    });
    keyboard.editActions(GLFW_KEY_D, [&](GLFWwindow *) {
        renderEngine->camera.position +=
          renderEngine->camera.right * renderEngine->frameTime * renderEngine->camera.speed;
    });
    keyboard.editActions(GLFW_KEY_SPACE, [&](GLFWwindow *) {
        renderEngine->camera.position +=
          renderEngine->camera.up * renderEngine->frameTime * renderEngine->camera.speed;
    });
    keyboard.editActions(GLFW_KEY_LEFT_SHIFT, [&](GLFWwindow *) {
        renderEngine->camera.position -=
          renderEngine->camera.up * renderEngine->frameTime * renderEngine->camera.speed;
    });
    keyboard.editActions({GLFW_KEY_LEFT_CONTROL, GLFW_PRESS}, [&](GLFWwindow *) {
        renderEngine->camera.speed *= 6.0;
    });
    keyboard.editActions({GLFW_KEY_LEFT_CONTROL, GLFW_RELEASE}, [&](GLFWwindow *) {
        renderEngine->camera.speed /= 6.0;
    });
    keyboard.editActions({GLFW_KEY_F11, GLFW_PRESS}, [&](GLFWwindow *) { renderEngine->queueToggleFullscreen(); });
    keyboard.editActions({GLFW_KEY_ESCAPE, GLFW_REPEAT}, [&](GLFWwindow *) {
        glfwSetWindowShouldClose(renderEngine->window, 1);
    });

    auto &am       = IE::Core::Core::getAssetManager();
    auto &fbxModel = am.createAspect<IERenderable>(
      "ancientStatue FBX",
      std::filesystem::path("res") / "assets" / "AncientStatue" / "models" / "ancientStatue.fbx"
    );
    auto ancientStatue =
      am.createAsset("ancientStatues", std::filesystem::path("res") / "assets" / "AncientStatue", &fbxModel);
    renderEngine->addAsset(&ancientStatue);

    renderEngine->camera.position = {0.0F, -2.0F, 1.0F};

    settings.logger.log("Beginning main loop.", IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_INFO);

    glfwSetTime(0.0);
    while (renderEngine->update()) {
        glfwPollEvents();
        keyboard.handleQueue();
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
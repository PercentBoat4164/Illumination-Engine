#include "Core/FileSystemModule/FileSystem.hpp"
#include "Core/LogModule/IELogger.hpp"
#include "Core/ThreadingModule/ThreadPool.hpp"
#include "GraphicsModule/IERenderEngine.hpp"
#include "InputModule/IEKeyboard.hpp"

#include <iostream>

int main() {
    IESettings           settings   = IESettings();
    IE::Core::FileSystem fileSystem = IE::Core::FileSystem("res/");

    // RenderEngine must be allocated on the heap.
    std::shared_ptr<IERenderEngine> renderEngine = std::make_shared<IERenderEngine>(&settings);

    IEKeyboard keyboard{renderEngine->window};
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
    keyboard.editActions({GLFW_KEY_F11, GLFW_PRESS}, [&](GLFWwindow *) { renderEngine->toggleFullscreen(); });
    keyboard.editActions({GLFW_KEY_ESCAPE, GLFW_REPEAT}, [&](GLFWwindow *) {
        glfwSetWindowShouldClose(renderEngine->window, 1);
    });

    fileSystem.addFile("assets/AncientStatue/models/ancientStatue.fbx");
    fileSystem.addFile("assets/AncientStatue/models/ancientStatue.obj");
    fileSystem.addFile("assets/AncientStatue/models/ancientStatue.glb");
    fileSystem.addFile("assets/DeepslateFloor/models/DeepslateFloor.fbx");

    IEWindowUser windowUser{std::shared_ptr<IERenderEngine>(renderEngine), &keyboard};
    glfwSetWindowUserPointer(renderEngine->window, &windowUser);

    std::shared_ptr<IEAsset> fbx = std::make_shared<IEAsset>();
    fbx->filename                = "res/assets/AncientStatue/models/ancientStatue.fbx";
    fbx->addAspect(new IERenderable{});
    fbx->position = {2, 1, 0};
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

    renderEngine->settings->logger.log(
      "Beginning main loop.",
      IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_INFO
    );

    glfwSetTime(0.0);
    while (renderEngine->update()) {
        glfwPollEvents();
        threadPool.submit([&] { keyboard.handleQueue(); });
    }
}
#include "Core/FileSystemModule/IEFileSystem.hpp"
#include "Core/ThreadingModule/ThreadPool/ThreadPool.hpp"
#include "GraphicsModule/IERenderEngine.hpp"
#include "InputModule/IEKeyboard.hpp"

int main() {
    IESettings                      settings = IESettings();
    std::shared_ptr<IERenderEngine> renderEngine =
      std::make_shared<IERenderEngine>(settings);  // RenderEngine must be allocated on the heap.

    IEKeyboard keyboard{renderEngine->window};
    keyboard.editActions(
      GLFW_KEY_W,
      [&](GLFWwindow *) {
          renderEngine->camera.position +=
            renderEngine->camera.front * renderEngine->frameTime * renderEngine->camera.speed;
      }
    );
    keyboard.editActions(
      GLFW_KEY_A,
      [&](GLFWwindow *) {
          renderEngine->camera.position -=
            renderEngine->camera.right * renderEngine->frameTime * renderEngine->camera.speed;
      }
    );
    keyboard.editActions(
      GLFW_KEY_S,
      [&](GLFWwindow *) {
          renderEngine->camera.position -=
            renderEngine->camera.front * renderEngine->frameTime * renderEngine->camera.speed;
      }
    );
    keyboard.editActions(
      GLFW_KEY_D,
      [&](GLFWwindow *) {
          renderEngine->camera.position +=
            renderEngine->camera.right * renderEngine->frameTime * renderEngine->camera.speed;
      }
    );
    keyboard.editActions(
      GLFW_KEY_SPACE,
      [&](GLFWwindow *) {
          renderEngine->camera.position +=
            renderEngine->camera.up * renderEngine->frameTime * renderEngine->camera.speed;
      }
    );
    keyboard.editActions(
      GLFW_KEY_LEFT_SHIFT,
      [&](GLFWwindow *) {
          renderEngine->camera.position -=
            renderEngine->camera.up * renderEngine->frameTime * renderEngine->camera.speed;
      }
    );
    keyboard.editActions(
      {GLFW_KEY_LEFT_CONTROL, GLFW_PRESS},
      [&](GLFWwindow *) { renderEngine->camera.speed *= 6.0; }
    );
    keyboard.editActions(
      {GLFW_KEY_LEFT_CONTROL, GLFW_RELEASE},
      [&](GLFWwindow *) { renderEngine->camera.speed /= 6.0; }
    );
    keyboard.editActions(
      {GLFW_KEY_F11, GLFW_PRESS},
      [&](GLFWwindow *) { renderEngine->toggleFullscreen(); }
    );
    keyboard.editActions({GLFW_KEY_ESCAPE, GLFW_REPEAT}, [&](GLFWwindow *) {
        glfwSetWindowShouldClose(renderEngine->window, 1);
    });

    IEWindowUser windowUser{std::shared_ptr<IERenderEngine>(renderEngine), &keyboard};
    glfwSetWindowUserPointer(renderEngine->window, &windowUser);

    IE::Core::ThreadPool threadPool{};

    std::shared_ptr<IEAsset> fbx(std::make_shared<IEAsset>());
    auto                     fbxFuture{threadPool.submit([&] {
        fbx->filename = "res/assets/AncientStatue/models/ancientStatue.fbx";
        fbx->position = {2, 1, 0};
        fbx->addAspect(std::make_shared<IERenderable>().get());
        renderEngine->addAsset(fbx);
    })};
    std::shared_ptr<IEAsset> obj(std::make_shared<IEAsset>());
    auto                     objFuture{threadPool.submit([&] {
        obj->filename = "res/assets/AncientStatue/models/ancientStatue.obj";
        obj->position = {0, 1, 0};
        obj->addAspect(std::make_shared<IERenderable>().get());
        renderEngine->addAsset(obj);
    })};
    std::shared_ptr<IEAsset> glb(std::make_shared<IEAsset>());
    auto                     glbFuture{threadPool.submit([&] {
        glb->filename = "res/assets/AncientStatue/models/ancientStatue.glb";
        glb->position = {-2, 1, 0};
        glb->addAspect(std::make_shared<IERenderable>().get());
        renderEngine->addAsset(glb);
    })};
    std::shared_ptr<IEAsset> floor(std::make_shared<IEAsset>());
    auto                     floorFuture{threadPool.submit([&] {
        floor->filename = "res/assets/DeepslateFloor/models/DeepslateFloor.fbx";
        floor->position = {0, 0, -1};
        floor->addAspect(std::make_shared<IERenderable>().get());
        renderEngine->addAsset(floor);
    })};

    renderEngine->camera.position = {0.0F, -2.0F, 1.0F};

    fbxFuture.wait();
    objFuture.wait();
    glbFuture.wait();
    floorFuture.wait();

    //    threadPool.shutdown();

    renderEngine->settings->m_logger.log("Beginning main loop.", ILLUMINATION_ENGINE_LOG_LEVEL_INFO);

    glfwSetTime(0.0);
    while (renderEngine->update()) {
        glfwPollEvents();
        keyboard.handleQueue();
    }
}
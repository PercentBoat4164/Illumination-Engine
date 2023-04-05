#include "Core/Core.hpp"
#include "IERenderEngine.hpp"
#include "InputModule/InputEngine.hpp"
#include "InputModule/Keyboard.hpp"

#include <iostream>
#include <SDL.h>
#include <thread>

IE::Core::Threading::CoroutineTask<void> illuminationEngine() {
    IESettings settings     = IESettings();
    auto      *renderEngine = IE::Core::Core::createEngine<IERenderEngine>("render engine", &settings);

    IE::Core::Core::getLogger()->log(std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id())));

    co_await IE::Core::Core::getThreadPool()->ensureThread(IE::Core::Threading::ThreadType::IE_THREAD_TYPE_MAIN_THREAD);
    IE::Core::Core::getLogger()->log(std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id())));
    // NOW EXECUTING ON THE MAIN THREAD.
    co_await IE::Core::Core::getThreadPool()->ensureThread(IE::Core::Threading::ThreadType::IE_THREAD_TYPE_WORKER_THREAD);
    IE::Core::Core::getLogger()->log(std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id())));
    // NOW EXECUTING ON A THREAD POOL MANAGED THREAD. Note that this thread is not *necessarily* the same as the worker thread before.

    // To submit an entire job use: `auto job = IE::Core::Core::getThreadPool()->submitToMainThread(illuminationEngine);`


    //    IE::Input::InputEngine inputEngine{renderEngine->window};
    //    IE::Input::Keyboard   *keyboard = inputEngine.getAspect("keyboard");
    //    keyboard->editActions(
    //      GLFW_KEY_W,
    //      [&](GLFWwindow *) {
    //          renderEngine->camera.position +=
    //            renderEngine->camera.front * renderEngine->frameTime * renderEngine->camera.speed;
    //      }
    //    );
    //    keyboard->editActions(
    //      GLFW_KEY_A,
    //      [&](GLFWwindow *) {
    //          renderEngine->camera.position -=
    //            renderEngine->camera.right * renderEngine->frameTime * renderEngine->camera.speed;
    //      }
    //    );
    //    keyboard->editActions(
    //      GLFW_KEY_S,
    //      [&](GLFWwindow *) {
    //          renderEngine->camera.position -=
    //            renderEngine->camera.front * renderEngine->frameTime * renderEngine->camera.speed;
    //      }
    //    );
    //    keyboard->editActions(
    //      GLFW_KEY_D,
    //      [&](GLFWwindow *) {
    //          renderEngine->camera.position +=
    //            renderEngine->camera.right * renderEngine->frameTime * renderEngine->camera.speed;
    //      }
    //    );
    //    keyboard->editActions(
    //      GLFW_KEY_SPACE,
    //      [&](GLFWwindow *) {
    //          renderEngine->camera.position +=
    //            renderEngine->camera.up * renderEngine->frameTime * renderEngine->camera.speed;
    //      }
    //    );
    //    keyboard->editActions(
    //      GLFW_KEY_LEFT_SHIFT,
    //      [&](GLFWwindow *) {
    //          renderEngine->camera.position -=
    //            renderEngine->camera.up * renderEngine->frameTime * renderEngine->camera.speed;
    //      }
    //    );
    //    keyboard->editActions(
    //      {GLFW_KEY_LEFT_CONTROL, GLFW_PRESS},
    //      [&](GLFWwindow *) { renderEngine->camera.speed *= 6.0; }
    //    );
    //    keyboard->editActions(
    //      {GLFW_KEY_LEFT_CONTROL, GLFW_RELEASE},
    //      [&](GLFWwindow *) { renderEngine->camera.speed /= 6.0; }
    //    );
    //    keyboard->editActions(
    //      {GLFW_KEY_F11, GLFW_PRESS},
    //      [&](GLFWwindow *) { renderEngine->queueToggleFullscreen(); }
    //    );
    //    keyboard->editActions({GLFW_KEY_ESCAPE, GLFW_REPEAT}, [&](GLFWwindow *) {
    //        glfwSetWindowShouldClose(renderEngine->window, 1);
    //    });

    //    std::shared_ptr<IEAsset> fbx(std::make_shared<IEAsset>());
    //    fbx->filename = "res/assets/AncientStatue/models/ancientStatue.fbx";
    //    fbx->position = {2, 1, 0};
    //    fbx->addAspect(new IERenderable{});
    //    renderEngine->addAsset(fbx);
    //    std::shared_ptr<IEAsset> obj = std::make_shared<IEAsset>();
    //    obj->filename                = "res/assets/AncientStatue/models/ancientStatue.obj";
    //    obj->addAspect(new IERenderable{});
    //    obj->position = {0, 1, 0};
    //    renderEngine->addAsset(obj);
    //    std::shared_ptr<IEAsset> glb = std::make_shared<IEAsset>();
    //    glb->filename                = "res/assets/AncientStatue/models/ancientStatue.glb";
    //    glb->addAspect(new IERenderable{});
    //    glb->position = {-2, 1, 0};
    //    renderEngine->addAsset(glb);
    std::shared_ptr<IEAsset> floor = std::make_shared<IEAsset>();
    floor->filename                = "res/assets/DeepslateFloor/models/DeepslateFloor.fbx";
    floor->addAspect(new IERenderable{});
    renderEngine->addAsset(floor);
    floor->position = {0, 0, -1};

    renderEngine->camera.position = {0.0F, -2.0F, 1.0F};

    settings.logger.log("Beginning main loop.", IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_INFO);

    SDL_Event e;

    //    glfwSetTime(0.0);
    while (renderEngine->update()) {
        //        glfwPollEvents();
        //        IE::Core::Core::getThreadPool()->submit([&] { keyboard->handleQueue(); });
        //@todo Add proper object destruction handling in the destructor of the RenderEngine (window, SDL_quit(),
        //etc...)
        while (SDL_PollEvent(&e))
            if (e.type == SDL_QUIT) co_return;
    }
}

int main(int argc, char **argv) {
    IE::Core::Core::getLogger()->log(std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id())));

    if (argc >= 1)
        IE::Core::Core::getInst(std::filesystem::path(argv[0]).parent_path().string());

    auto job = IE::Core::Core::getThreadPool()->submit(illuminationEngine);

    IE::Core::Core::getThreadPool()->startMainThreadLoop();

    job->wait();

    return 0;
}
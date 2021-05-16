#include <iostream>
#include "PhysicsEngine/Modules/RigidBody.hpp"
#include "PhysicsEngine/Core/World.hpp"
#ifdef CRYSTAL_ENGINE_VULKAN
#include "GraphicsEngine/Vulkan/Asset.hpp"
#include "GraphicsEngine/Vulkan/VulkanRenderEngineRasterizer.hpp"
#ifdef CRYSTAL_ENGINE_VULKAN_RAY_TRACING
#include "GraphicsEngine/Vulkan/VulkanRenderEngineRayTracer.hpp"
#endif
#endif
#ifdef CRYSTAL_ENGINE_OPENGL
#include "GraphicsEngine/OpenGL/OpenGLSettings.hpp"
#include "GraphicsEngine/OpenGL/OpenGLRenderEngine.hpp"
#endif

#ifdef CRYSTAL_ENGINE_VULKAN
void cursorCallback(GLFWwindow *window, double xOffset, double yOffset) {
    auto vulkanRenderEngineRasterizer = static_cast<VulkanRenderEngineRasterizer *>(glfwGetWindowUserPointer(window));
    xOffset *= vulkanRenderEngineRasterizer->settings.mouseSensitivity;
    yOffset *= vulkanRenderEngineRasterizer->settings.mouseSensitivity;
    vulkanRenderEngineRasterizer->camera.yaw -= (float) xOffset;
    vulkanRenderEngineRasterizer->camera.pitch -= (float) yOffset;
    if (vulkanRenderEngineRasterizer->camera.pitch > 89.99f) { vulkanRenderEngineRasterizer->camera.pitch = 89.99f; }
    if (vulkanRenderEngineRasterizer->camera.pitch < -89.99f) { vulkanRenderEngineRasterizer->camera.pitch = -89.99f; }
    glfwSetCursorPos(window, 0, 0);
}

void windowPositionCallback(GLFWwindow *window, int xPos, int yPos) {
    auto vulkanRenderEngineRasterizer = static_cast<VulkanRenderEngineRasterizer *>(glfwGetWindowUserPointer(window));
    if (!vulkanRenderEngineRasterizer->settings.fullscreen) { vulkanRenderEngineRasterizer->settings.windowPosition = {xPos, yPos}; }
}

#endif

int main(int argc, char **argv) {
    glfwWindowHint(GLFW_MAXIMIZED, 1);
    std::string selection;
    char input;
    if (argc > 1) { input = *argv[1]; }
    else {
        std::cout << "'v': Run Vulkan render engine\n'o': Run OpenGL render engine\n'p': Run Physics engine\n";
        std::cin >> selection;
        input = selection[0];
    }
#ifdef CRYSTAL_ENGINE_VULKAN
    if (input == 'v') {
        try {
            VulkanRenderEngineRasterizer renderEngine = VulkanRenderEngineRasterizer();
            glfwSetWindowPosCallback(renderEngine.window, windowPositionCallback);
            renderEngine.camera.position = {10, 11, 2};
            RigidBody sphereBody(new Sphere(glm::vec3(0, 0, 0), 1.0f));
            RigidBody sphereBody2(new Sphere(glm::vec3(0, 0, 0), 1.0f));
            sphereBody.position = glm::vec3(0, 1, 1);
            sphereBody2.position = glm::vec3(10, 0, 1);
            sphereBody.mass = 1;
            sphereBody2.mass = 1;
            sphereBody.applyImpulse(glm::vec3(-0.005, 0, 0));
            sphereBody2.applyImpulse(glm::vec3(-0.01, 0, 0));
            World world{};
            world.addActiveBody(&sphereBody);
            world.addActiveBody(&sphereBody2);
            Asset leatherBall = Asset("Models/icosphere.obj", {"Models/sphere_diffuse.png"}, {"Shaders/vertexShader.vert", "Shaders/fragmentShader.frag"}, {0, 0, 0}, {0, 0, 0});
            renderEngine.uploadAsset(&leatherBall, true);
            Asset lightBall = Asset("Models/icosphere.obj", {"Models/sphere_normal.png"}, {"Shaders/vertexShader.vert", "Shaders/fragmentShader.frag"}, {10, 0, 0}, {0, 0, 0});
            renderEngine.uploadAsset(&lightBall, true);
            Asset ground = Asset{"Models/quad.obj",  {"Models/quad_Color.png"}, {"Shaders/vertexShader.vert", "Shaders/fragmentShader.frag"}, {0, 0, 0}, {90, 0, 0}, {100, 100, 0}};
            renderEngine.uploadAsset(&ground, true);
            double lastTab{0};
            double lastF2{0};
            double lastEsc{0};
            double lastCursorPosX{0};
            double lastCursorPosY{0};
            bool captureInput{};
            std::vector<float> recordedFPS{};
            float recordedFPSCount{200};
            recordedFPS.resize((size_t)recordedFPSCount);
            while (renderEngine.update()) {                
                //Process inputs
                glfwPollEvents();
                float velocity = renderEngine.frameTime * renderEngine.settings.movementSpeed;
                if ((bool)glfwGetKey(renderEngine.window, GLFW_KEY_F1)) {
                    for (Asset *asset : renderEngine.assets) { asset->reloadAsset(); }
                    renderEngine.updateSettings(false);
                } if ((bool)glfwGetKey(renderEngine.window, GLFW_KEY_F2) & (glfwGetTime() - lastF2 > .2)) {
                    renderEngine.settings.fullscreen = !renderEngine.settings.fullscreen;
                    renderEngine.updateSettings(true);
                    lastF2 = glfwGetTime();
                } if ((bool)glfwGetKey(renderEngine.window, GLFW_KEY_1)) {
                    renderEngine.settings.msaaSamples = VK_SAMPLE_COUNT_1_BIT;
                    renderEngine.updateSettings(true);
                } if ((bool)glfwGetKey(renderEngine.window, GLFW_KEY_8)) {
                    renderEngine.settings.msaaSamples = VK_SAMPLE_COUNT_8_BIT;
                    renderEngine.updateSettings(true);
                } if ((bool)glfwGetKey(renderEngine.window, GLFW_KEY_LEFT_CONTROL) & captureInput) { velocity *= 6; }
                if ((bool)glfwGetKey(renderEngine.window, GLFW_KEY_W) & captureInput) { renderEngine.camera.position += renderEngine.camera.front * velocity; }
                if ((bool)glfwGetKey(renderEngine.window, GLFW_KEY_A) & captureInput) { renderEngine.camera.position -= renderEngine.camera.right * velocity; }
                if ((bool)glfwGetKey(renderEngine.window, GLFW_KEY_S) & captureInput) { renderEngine.camera.position -= renderEngine.camera.front * velocity; }
                if ((bool)glfwGetKey(renderEngine.window, GLFW_KEY_D) & captureInput) { renderEngine.camera.position += renderEngine.camera.right * velocity; }
                if ((bool)glfwGetKey(renderEngine.window, GLFW_KEY_LEFT_SHIFT) & captureInput) { renderEngine.camera.position -= renderEngine.camera.up * velocity; }
                if ((bool)glfwGetKey(renderEngine.window, GLFW_KEY_SPACE) & captureInput) { renderEngine.camera.position += renderEngine.camera.up * velocity; }
                if ((bool)glfwGetKey(renderEngine.window, GLFW_KEY_TAB) & (glfwGetTime() - lastTab > .2)) {
                    int mode{glfwGetInputMode(renderEngine.window, GLFW_CURSOR)};
                    if (mode == GLFW_CURSOR_DISABLED) {
                        glfwGetCursorPos(renderEngine.window, &lastCursorPosX, &lastCursorPosY);
                        glfwSetInputMode(renderEngine.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                        glfwSetCursorPosCallback(renderEngine.window, nullptr);
                    } else if (mode == GLFW_CURSOR_NORMAL) {
                        if (glfwGetWindowAttrib(renderEngine.window, GLFW_HOVERED)) {
                            glfwSetInputMode(renderEngine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                            glfwSetCursorPos(renderEngine.window, 0, 0);
                            glfwSetCursorPosCallback(renderEngine.window, cursorCallback);
                            glfwSetCursorPos(renderEngine.window, lastCursorPosX, lastCursorPosY);
                        } else {
                            glfwSetCursorPos(renderEngine.window, 0, 0);
                            glfwSetInputMode(renderEngine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                            glfwSetCursorPosCallback(renderEngine.window, cursorCallback);
                        }
                    }
                    lastTab = glfwGetTime();
                    captureInput = !captureInput;
                } if ((bool)glfwGetKey(renderEngine.window, GLFW_KEY_ESCAPE) & (glfwGetTime() - lastEsc > .2)) {
                    if (!captureInput & !renderEngine.settings.fullscreen) { glfwSetWindowShouldClose(renderEngine.window, 1); }
                    if (renderEngine.settings.fullscreen) {
                        renderEngine.settings.fullscreen = false;
                        renderEngine.updateSettings(true);
                    } if (glfwGetInputMode(renderEngine.window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
                        glfwGetCursorPos(renderEngine.window, &lastCursorPosX, &lastCursorPosY);
                        glfwSetInputMode(renderEngine.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                        glfwSetCursorPosCallback(renderEngine.window, nullptr);
                        captureInput = false;
                    }
                    lastEsc = glfwGetTime();
                }
                //update framerate gathered over past 'recordedFPSCount' frames
                recordedFPS[(size_t)fmod((float)renderEngine.frameNumber, recordedFPSCount)] = 1 / renderEngine.frameTime;
                int sum{0};
                std::for_each(recordedFPS.begin(), recordedFPS.end(), [&] (int n) { sum += n; });
                glfwSetWindowTitle(renderEngine.window, (std::string("CrystalEngine - ") + std::to_string((float)sum / recordedFPSCount)).c_str());

                //move objects
                world.step(1/(sum / recordedFPSCount));
                if(world.checkCollision(sphereBody, sphereBody2, 0)) {
                    std::cout << "\nCollision!";
                    world.semiElasticCollide(&sphereBody, &sphereBody2);
                }
                leatherBall.position = sphereBody.position;
                lightBall.position = sphereBody2.position;
            }
            renderEngine.cleanUp();
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
#endif
#ifdef CRYSTAL_ENGINE_OPENGL
    if (input == 'o') {
        try {
            OpenGLRenderEngine renderEngine = OpenGLRenderEngine();
            glfwSwapInterval(1);
            std::vector<float> recordedFPS{};
            float recordedFPSCount{200};
            recordedFPS.resize((size_t)recordedFPSCount);
            while (renderEngine.update() != 1) {
                glfwPollEvents();
                //update framerate gathered over past 'recordedFPSCount' frames
                recordedFPS[(size_t)fmod((float)renderEngine.frameNumber, recordedFPSCount)] = 1 / renderEngine.frameTime;
                int sum{0};
                std::for_each(recordedFPS.begin(), recordedFPS.end(), [&] (int n) { sum += n; });
                glfwSetWindowTitle(renderEngine.window, (std::string("CrystalEngine - ") + std::to_string((float)sum / recordedFPSCount)).c_str());
            }
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
#endif
}
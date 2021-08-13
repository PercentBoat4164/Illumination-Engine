#include <iostream>
#include <cmath>

#ifdef ILLUMINATION_ENGINE_VULKAN
#include "GraphicsEngine/Vulkan/vulkanRenderEngine.hpp"
#endif
#ifdef ILLUMINATION_ENGINE_OPENGL
#include "GraphicsEngine/OpenGL/openglRenderEngine.hpp"
#endif

#ifdef ILLUMINATION_ENGINE_VULKAN
void vulkanCursorCallback(GLFWwindow *window, double xOffset, double yOffset) {
    auto pRenderEngine = static_cast<VulkanRenderEngine *>(glfwGetWindowUserPointer(window));
    xOffset *= pRenderEngine->settings.mouseSensitivity;
    yOffset *= pRenderEngine->settings.mouseSensitivity;
    pRenderEngine->camera.yaw -= (float) xOffset;
    pRenderEngine->camera.pitch -= (float) yOffset;
    if (pRenderEngine->camera.pitch > 89.99f) { pRenderEngine->camera.pitch = 89.99f; }
    if (pRenderEngine->camera.pitch < -89.99f) { pRenderEngine->camera.pitch = -89.99f; }
    pRenderEngine->camera.front = glm::normalize(glm::vec3{cos(glm::radians(pRenderEngine->camera.yaw)) * cos(glm::radians(pRenderEngine->camera.pitch)), sin(glm::radians(pRenderEngine->camera.yaw)) * cos(glm::radians(pRenderEngine->camera.pitch)), sin(glm::radians(pRenderEngine->camera.pitch))});
    glfwSetCursorPos(window, 0, 0);
}

void vulkanRasterizerWindowPositionCallback(GLFWwindow *window, int xPos, int yPos) {
    auto pRenderEngine = static_cast<VulkanRenderEngine *>(glfwGetWindowUserPointer(window));
    if (!pRenderEngine->settings.fullscreen) { pRenderEngine->settings.windowPosition = {xPos, yPos}; }
}
#endif

#ifdef ILLUMINATION_ENGINE_OPENGL
void openglCursorCallback(GLFWwindow *window, double xOffset, double yOffset) {
    auto pRenderEngine = static_cast<OpenGLRenderEngine *>(glfwGetWindowUserPointer(window));
    xOffset *= pRenderEngine->settings.mouseSensitivity;
    yOffset *= pRenderEngine->settings.mouseSensitivity;
    pRenderEngine->camera.yaw -= (float) xOffset;
    pRenderEngine->camera.pitch -= (float) yOffset;
    if (pRenderEngine->camera.pitch > 89.99f) { pRenderEngine->camera.pitch = 89.99f; }
    if (pRenderEngine->camera.pitch < -89.99f) { pRenderEngine->camera.pitch = -89.99f; }
    pRenderEngine->camera.front = glm::normalize(glm::vec3{cos(glm::radians(pRenderEngine->camera.yaw)) * cos(glm::radians(pRenderEngine->camera.pitch)), sin(glm::radians(pRenderEngine->camera.yaw)) * cos(glm::radians(pRenderEngine->camera.pitch)), sin(glm::radians(pRenderEngine->camera.pitch))});
    glfwSetCursorPos(window, 0, 0);
}

void openglWindowPositionCallback(GLFWwindow *window, int xPos, int yPos) {
    auto pOpenGlRenderEngine = static_cast<OpenGLRenderEngine *>(glfwGetWindowUserPointer(window));
    if (!pOpenGlRenderEngine->settings.fullscreen) { pOpenGlRenderEngine->settings.windowPosition = {xPos, yPos}; }
}
#endif

int main(int argc, char **argv) {
    std::string selection;
    std::string input;
    if (argc > 1) { input = *argv[1]; }
    else {
        std::cout << "'v': Run Vulkan render engine\n'o': Run OpenGL render engine\n";
        std::cin >> selection;
        input = selection[0];
    }
#ifdef ILLUMINATION_ENGINE_VULKAN
    if (input == "v") {
        try {
            VulkanRenderEngine renderEngine{};
            glfwSetWindowPosCallback(renderEngine.window, vulkanRasterizerWindowPositionCallback);
            renderEngine.camera.position = {0, 0, 2};
            VulkanRenderable cube{&renderEngine.renderEngineLink, "res/Models/Cube/cube.obj"};
            VulkanRenderable quad{&renderEngine.renderEngineLink, "res/Models/Quad/quad.obj"};
            VulkanRenderable rock{&renderEngine.renderEngineLink, "res/Models/Rock/rock.obj"};
            VulkanRenderable statue{&renderEngine.renderEngineLink, "res/Models/AncientStatue/ancientStatue.obj"};
            VulkanRenderable ball{&renderEngine.renderEngineLink, "res/Models/Sphere/sphere.obj"};
            VulkanRenderable backpack{&renderEngine.renderEngineLink, "res/Models/Backpack/Survival_BackPack_2.fbx"};
            renderEngine.loadRenderable(&cube);
            renderEngine.loadRenderable(&quad);
            renderEngine.loadRenderable(&rock);
            renderEngine.loadRenderable(&statue);
            renderEngine.loadRenderable(&ball);
            renderEngine.loadRenderable(&backpack);
            double lastKey{0};
            double lastCursorPosX{0};
            double lastCursorPosY{0};
            bool captureInput{};
            double tempTime;
            while (renderEngine.update()) {
                glfwPollEvents();
                float maxVelocity = renderEngine.frameTime * renderEngine.settings.movementSpeed;
                if (glfwGetKey(renderEngine.window, GLFW_KEY_F1)) {
                    tempTime = glfwGetTime();
                    renderEngine.reloadRenderables();
                    glfwSetTime(tempTime);
                } if (glfwGetKey(renderEngine.window, GLFW_KEY_F2) & (glfwGetTime() - lastKey > .2)) {
                    renderEngine.settings.fullscreen = !renderEngine.settings.fullscreen;
                    renderEngine.handleFullscreenSettingsChange();
                    lastKey = glfwGetTime();
                } if (glfwGetKey(renderEngine.window, GLFW_KEY_1)) {
                    renderEngine.settings.msaaSamples = VK_SAMPLE_COUNT_1_BIT;
                    tempTime = glfwGetTime();
                    renderEngine.createSwapchain(true);
                    glfwSetTime(tempTime);
                } if (glfwGetKey(renderEngine.window, GLFW_KEY_8)) {
                    renderEngine.settings.msaaSamples = VK_SAMPLE_COUNT_8_BIT;
                    tempTime = glfwGetTime();
                    renderEngine.createSwapchain(true);
                    glfwSetTime(tempTime);
                } if (glfwGetKey(renderEngine.window, GLFW_KEY_M)) {
                    renderEngine.settings.mipMapping ^= 1;
                    tempTime = glfwGetTime();
                    renderEngine.reloadRenderables();
                    glfwSetTime(tempTime);
                } if (glfwGetKey(renderEngine.window, GLFW_KEY_R) & (glfwGetTime() - lastKey > .2)) {
                    if (renderEngine.renderEngineLink.enabledPhysicalDeviceInfo.rayTracing) {
                        renderEngine.settings.rayTracing ^= 1;
                        tempTime = glfwGetTime();
                        renderEngine.reloadRenderables();
                        glfwSetTime(tempTime);
                    } else { std::cout << "This machine does not support the Vulkan extensions required for ray tracing." << std::endl; }
                    lastKey = glfwGetTime();
                } if (glfwGetKey(renderEngine.window, GLFW_KEY_LEFT_CONTROL) & captureInput) { maxVelocity *= 6; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_W) & captureInput) { renderEngine.camera.position += renderEngine.camera.front * maxVelocity; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_A) & captureInput) { renderEngine.camera.position -= renderEngine.camera.right * maxVelocity; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_S) & captureInput) { renderEngine.camera.position -= renderEngine.camera.front * maxVelocity; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_D) & captureInput) { renderEngine.camera.position += renderEngine.camera.right * maxVelocity; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_LEFT_SHIFT) & captureInput) { renderEngine.camera.position -= renderEngine.camera.up * maxVelocity; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_SPACE) & captureInput) { renderEngine.camera.position += renderEngine.camera.up * maxVelocity; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_TAB) & (glfwGetTime() - lastKey > .2)) {
                    int mode{glfwGetInputMode(renderEngine.window, GLFW_CURSOR)};
                    if (mode == GLFW_CURSOR_DISABLED) {
                        glfwGetCursorPos(renderEngine.window, &lastCursorPosX, &lastCursorPosY);
                        glfwSetInputMode(renderEngine.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                        glfwSetCursorPosCallback(renderEngine.window, nullptr);
                    } else if (mode == GLFW_CURSOR_NORMAL) {
                        if (glfwGetWindowAttrib(renderEngine.window, GLFW_HOVERED)) {
                            glfwSetInputMode(renderEngine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                            glfwSetCursorPos(renderEngine.window, 0, 0);
                            glfwSetCursorPosCallback(renderEngine.window, vulkanCursorCallback);
                            glfwSetCursorPos(renderEngine.window, lastCursorPosX, lastCursorPosY);
                        } else {
                            glfwSetCursorPos(renderEngine.window, 0, 0);
                            glfwSetInputMode(renderEngine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                            glfwSetCursorPosCallback(renderEngine.window, vulkanCursorCallback);
                        }
                    }
                    lastKey = glfwGetTime();
                    captureInput = !captureInput;
                } if (glfwGetKey(renderEngine.window, GLFW_KEY_ESCAPE) & (glfwGetTime() - lastKey > .2)) {
                    if (!captureInput & !renderEngine.settings.fullscreen) { glfwSetWindowShouldClose(renderEngine.window, 1); }
                    else if (glfwGetInputMode(renderEngine.window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
                        glfwGetCursorPos(renderEngine.window, &lastCursorPosX, &lastCursorPosY);
                        glfwSetInputMode(renderEngine.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                        glfwSetCursorPosCallback(renderEngine.window, nullptr);
                        captureInput = false;
                    } else if (renderEngine.settings.fullscreen) {
                        renderEngine.settings.fullscreen = false;
                        renderEngine.handleFullscreenSettingsChange();
                    }
                    lastKey = glfwGetTime();
                }
                cube.position = {10 * cos(0.5f * glfwGetTime()), 10 * sin(0.5f * glfwGetTime()), 1};
                ball.position = {10 * -cos(0.5f * glfwGetTime()), 10 * -sin(0.5f * glfwGetTime()), 1};
                backpack.position = {0, 7, 0};
                statue.position = {7, 0, 0};
                rock.position = {-7, 0, 0};
                quad.scale = {100, 100, 100};
                quad.rotation = {90, 0, 0};
            }
            renderEngine.destroy();
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
#endif
#ifdef ILLUMINATION_ENGINE_OPENGL
    if (input == "o") {
        try {
            OpenGLRenderEngine renderEngine{};
            glfwSetWindowPosCallback(renderEngine.window, openglWindowPositionCallback);
            renderEngine.camera.position = {0, 0, 2};
            OpenGLRenderable cube{"res/Models/Cube/cube.obj"};
            OpenGLRenderable quad{"res/Models/Quad/quad.obj"};
            OpenGLRenderable rock{"res/Models/Rock/rock.obj"};
            OpenGLRenderable statue{"res/Models/AncientStatue/ancientStatue.obj"};
            OpenGLRenderable ball{"res/Models/Sphere/sphere.obj"};
            OpenGLRenderable backpack{"res/Models/Backpack/Survival_BackPack_2.fbx"};
            renderEngine.loadRenderable(&cube);
            renderEngine.loadRenderable(&quad);
            renderEngine.loadRenderable(&rock);
            renderEngine.loadRenderable(&statue);
            renderEngine.loadRenderable(&ball);
            renderEngine.loadRenderable(&backpack);
            double lastKey{0};
            double lastCursorPosX{0};
            double lastCursorPosY{0};
            bool captureInput{};
            double tempTime;
            while (renderEngine.update() != 1) {
                glfwPollEvents();
                float maxVelocity = (float)renderEngine.frameTime * (float)renderEngine.settings.movementSpeed;
                if (glfwGetKey(renderEngine.window, GLFW_KEY_F1)) {
                    tempTime = glfwGetTime();
                    renderEngine.reloadRenderables();
                    glfwSetTime(tempTime);
                } if (glfwGetKey(renderEngine.window, GLFW_KEY_F2) & (glfwGetTime() - lastKey > .2)) {
                    renderEngine.settings.fullscreen ^= 1;
                    renderEngine.updateSettings();
                    lastKey = glfwGetTime();
                } if (glfwGetKey(renderEngine.window, GLFW_KEY_M)) {
                    renderEngine.settings.mipMapping ^= 1;
                    tempTime = glfwGetTime();
                    renderEngine.reloadRenderables();
                    glfwSetTime(tempTime);
                } if (glfwGetKey(renderEngine.window, GLFW_KEY_R) & (glfwGetTime() - lastKey > .2)) {
                    std::cout << "Ray tracing is not supported in OpenGL ... yet." << std::endl; // Some offline raytracer might be implemented at some point. It will require version 3.3 or greater.
                    lastKey = glfwGetTime();
                } if (glfwGetKey(renderEngine.window, GLFW_KEY_LEFT_CONTROL) & captureInput) { maxVelocity *= 6; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_W) & captureInput) { renderEngine.camera.position += renderEngine.camera.front * maxVelocity; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_A) & captureInput) { renderEngine.camera.position -= renderEngine.camera.right * maxVelocity; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_S) & captureInput) { renderEngine.camera.position -= renderEngine.camera.front * maxVelocity; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_D) & captureInput) { renderEngine.camera.position += renderEngine.camera.right * maxVelocity; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_LEFT_SHIFT) & captureInput) { renderEngine.camera.position -= renderEngine.camera.up * maxVelocity; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_SPACE) & captureInput) { renderEngine.camera.position += renderEngine.camera.up * maxVelocity; }
                if (glfwGetKey(renderEngine.window, GLFW_KEY_TAB) & (glfwGetTime() - lastKey > .2)) {
                    int mode{glfwGetInputMode(renderEngine.window, GLFW_CURSOR)};
                    if (mode == GLFW_CURSOR_DISABLED) {
                        glfwGetCursorPos(renderEngine.window, &lastCursorPosX, &lastCursorPosY);
                        glfwSetInputMode(renderEngine.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                        glfwSetCursorPosCallback(renderEngine.window, nullptr);
                    } else if (mode == GLFW_CURSOR_NORMAL) {
                        if (glfwGetWindowAttrib(renderEngine.window, GLFW_HOVERED)) {
                            glfwSetInputMode(renderEngine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                            glfwSetCursorPos(renderEngine.window, 0, 0);
                            glfwSetCursorPosCallback(renderEngine.window, openglCursorCallback);
                            glfwSetCursorPos(renderEngine.window, lastCursorPosX, lastCursorPosY);
                        } else {
                            glfwSetCursorPos(renderEngine.window, 0, 0);
                            glfwSetInputMode(renderEngine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                            glfwSetCursorPosCallback(renderEngine.window, openglCursorCallback);
                        }
                    }
                    lastKey = glfwGetTime();
                    captureInput = !captureInput;
                } if (glfwGetKey(renderEngine.window, GLFW_KEY_ESCAPE) & (glfwGetTime() - lastKey > .2)) {
                    if (!captureInput & !renderEngine.settings.fullscreen) { glfwSetWindowShouldClose(renderEngine.window, 1); }
                    else if (glfwGetInputMode(renderEngine.window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
                        glfwGetCursorPos(renderEngine.window, &lastCursorPosX, &lastCursorPosY);
                        glfwSetInputMode(renderEngine.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                        glfwSetCursorPosCallback(renderEngine.window, nullptr);
                        captureInput = false;
                    } else if (renderEngine.settings.fullscreen) {
                        renderEngine.settings.fullscreen = false;
                        renderEngine.updateSettings();
                    }
                    lastKey = glfwGetTime();
                }
                cube.position = {10 * cos(0.5f * glfwGetTime()), 10 * sin(0.5f * glfwGetTime()), 1};
                ball.position = {10 * -cos(0.5f * glfwGetTime()), 10 * -sin(0.5f * glfwGetTime()), 1};
                backpack.position = {0, 7, 0};
                statue.position = {7, 0, 0};
                rock.position = {-7, 0, 0};
                quad.scale = {100, 100, 100};
                quad.rotation = {90, 0, 0};
            }
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
#endif
}
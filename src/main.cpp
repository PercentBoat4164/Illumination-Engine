#include <iostream>
#include <cmath>

#ifdef ILLUMINATION_ENGINE_VULKAN
#include "GraphicsEngine/Vulkan/vulkanRenderEngineRasterizer.hpp"
#ifdef ILLUMINATION_ENGINE_VULKAN_RAY_TRACING
#include "GraphicsEngine/Vulkan/vulkanRenderEngineRayTracer.hpp"
#endif
#endif
#ifdef ILLUMINATION_ENGINE_OPENGL
#include "GraphicsEngine/OpenGL/openglRenderEngine.hpp"
#endif

#ifdef ILLUMINATION_ENGINE_VULKAN
void vulkanRasterizerCursorCallback(GLFWwindow *window, double xOffset, double yOffset) {
    auto pRenderEngine = static_cast<VulkanRenderEngineRasterizer *>(glfwGetWindowUserPointer(window));
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
    auto pVulkanRenderEngineRasterizer = static_cast<VulkanRenderEngineRasterizer *>(glfwGetWindowUserPointer(window));
    if (!pVulkanRenderEngineRasterizer->settings.fullscreen) { pVulkanRenderEngineRasterizer->settings.windowPosition = {xPos, yPos}; }
}
#ifdef ILLUMINATION_ENGINE_VULKAN_RAY_TRACING
void vulkanRayTracerCursorCallback(GLFWwindow *window, double xOffset, double yOffset) {
    auto pRenderEngine = static_cast<VulkanRenderEngineRayTracer *>(glfwGetWindowUserPointer(window));
    xOffset *= pRenderEngine->settings.mouseSensitivity;
    yOffset *= pRenderEngine->settings.mouseSensitivity;
    pRenderEngine->camera.yaw -= (float) xOffset;
    pRenderEngine->camera.pitch -= (float) yOffset;
    if (pRenderEngine->camera.pitch > 89.99f) { pRenderEngine->camera.pitch = 89.99f; }
    if (pRenderEngine->camera.pitch < -89.99f) { pRenderEngine->camera.pitch = -89.99f; }
    pRenderEngine->camera.front = glm::normalize(glm::vec3{cos(glm::radians(pRenderEngine->camera.yaw)) * cos(glm::radians(pRenderEngine->camera.pitch)), sin(glm::radians(pRenderEngine->camera.yaw)) * cos(glm::radians(pRenderEngine->camera.pitch)), sin(glm::radians(pRenderEngine->camera.pitch))});
    glfwSetCursorPos(window, 0, 0);
}

void vulkanRayTracerWindowPositionCallback(GLFWwindow *window, int xPos, int yPos) {
    auto pVulkanRenderEngineRasterizer = static_cast<VulkanRenderEngineRayTracer *>(glfwGetWindowUserPointer(window));
    if (!pVulkanRenderEngineRasterizer->settings.fullscreen) { pVulkanRenderEngineRasterizer->settings.windowPosition = {xPos, yPos}; }
}
#endif
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
    glfwWindowHint(GLFW_MAXIMIZED, 1);
    std::string selection;
    std::string input;
    if (argc > 1) { input = *argv[1]; }
    else {
        std::cout << "'v': Run Vulkan rasterization engine\n'r': Run Vulkan raytracing engine\n'o': Run OpenGL render engine\n'p': Run Physics engine\n";
        std::cin >> selection;
        input = selection[0];
    }
#ifdef ILLUMINATION_ENGINE_VULKAN
    if (input == "v") {
        try {
            VulkanRenderEngineRasterizer renderEngine = VulkanRenderEngineRasterizer(nullptr, false);
            glfwSetWindowPosCallback(renderEngine.window, vulkanRasterizerWindowPositionCallback);
            renderEngine.camera.position = {0, 2, 0};
            VulkanRenderable cube = VulkanRenderable("res/Models/cube.obj", {"res/Models/cube.png"}, {"res/Shaders/VulkanRasterizationShaders/vertexShader.vert", "res/Shaders/VulkanRasterizationShaders/fragmentShader.frag"}, {0, 0, 0}, {0, 0, 0});
            VulkanRenderable quad = VulkanRenderable("res/Models/quad.obj", {"res/Models/quad_Color.png"}, {"res/Shaders/VulkanRasterizationShaders/vertexShader.vert", "res/Shaders/VulkanRasterizationShaders/fragmentShader.frag"}, {0, 0, 0}, {90, 0, 0}, {100, 100, 0});
            VulkanRenderable vikingRoom = VulkanRenderable("res/Models/vikingRoom.obj", {"res/Models/vikingRoom.png"}, {"res/Shaders/VulkanRasterizationShaders/vertexShader.vert", "res/Shaders/VulkanRasterizationShaders/fragmentShader.frag"}, {0, 0, 0}, {0, 0, 0}, {5, 5, 5});
            VulkanRenderable statue = VulkanRenderable("res/Models/ancientStatue.obj", {"res/Models/ancientStatue.png"}, {"res/Shaders/VulkanRasterizationShaders/vertexShader.vert", "res/Shaders/VulkanRasterizationShaders/fragmentShader.frag"}, {0, 0, 0}, {0, 0, 0});
            VulkanRenderable ball = VulkanRenderable("res/Models/sphere.obj", {"res/Models/sphere_diffuse.png"}, {"res/Shaders/VulkanRasterizationShaders/vertexShader.vert", "res/Shaders/VulkanRasterizationShaders/fragmentShader.frag"});
            renderEngine.uploadRenderable(&cube, true);
            renderEngine.uploadRenderable(&quad, true);
            renderEngine.uploadRenderable(&vikingRoom, true);
            renderEngine.uploadRenderable(&statue, true);
            renderEngine.uploadRenderable(&ball, true);
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
                    for (VulkanRenderable *renderable : renderEngine.renderables) { renderable->reloadRenderable(); }
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
                            glfwSetCursorPosCallback(renderEngine.window, vulkanRasterizerCursorCallback);
                            glfwSetCursorPos(renderEngine.window, lastCursorPosX, lastCursorPosY);
                        } else {
                            glfwSetCursorPos(renderEngine.window, 0, 0);
                            glfwSetInputMode(renderEngine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                            glfwSetCursorPosCallback(renderEngine.window, vulkanRasterizerCursorCallback);
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
                //move renderables
                cube.position = {10 * cos(3 * glfwGetTime()), 10 * sin(3 * glfwGetTime()), 1};
                ball.position = {10 * -cos(3 * glfwGetTime()), 10 * -sin(3 * glfwGetTime()), 1};
                //update framerate gathered over past 'recordedFPSCount' frames
                recordedFPS[(size_t)std::fmod((float)renderEngine.frameNumber, recordedFPSCount)] = 1 / renderEngine.frameTime;
                int sum{0};
                std::for_each(recordedFPS.begin(), recordedFPS.end(), [&] (int n) { sum += n; });
                glfwSetWindowTitle(renderEngine.window, (std::string("Illumination Engine - ") + std::to_string((float)sum / recordedFPSCount)).c_str());
            }
            renderEngine.destroy();
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
    #ifdef ILLUMINATION_ENGINE_VULKAN_RAY_TRACING
        if (input == "r") {
            try {
                VulkanRenderEngineRayTracer renderEngine = VulkanRenderEngineRayTracer(nullptr, true);
                glfwSetWindowPosCallback(renderEngine.window, vulkanRayTracerWindowPositionCallback);
                renderEngine.camera.position = {0, 10, 0};
                renderEngine.settings.rayTracing = true;
//                VulkanRenderable quad = VulkanRenderable("res/Models/quad.obj", {"res/Models/quad_Color.png"}, {"res/Shaders/VulkanRayTracingShaders/vertexShader.vert", "res/Shaders/VulkanRayTracingShaders/callable.rcall"}, {0, 0, 0}, {90, 0, 0}, {100, 100, 1});
//                renderEngine.uploadRenderable(&quad, true);
//                VulkanRenderable cube = VulkanRenderable("res/Models/cube.obj", {"res/Models/cube.png"}, {"res/Shaders/VulkanRayTracingShaders/vertexShader.vert", "res/Shaders/VulkanRayTracingShaders/callable.rcall"}, {0, 0, 0}, {0, 0, 0}, {1, 1, 1});
//                renderEngine.uploadRenderable(&cube, true);
//                VulkanRenderable viking_room = VulkanRenderable("res/Models/vikingRoom.obj", {"res/Models/vikingRoom.png"}, {"res/Shaders/VulkanRayTracingShaders/vertexShader.vert", "res/Shaders/VulkanRayTracingShaders/callable.rcall"}, {0, 0, 0}, {0, 0, 0}, {1, 1, 1});
//                renderEngine.uploadRenderable(&viking_room, true);
                VulkanRenderable ancientStatue = VulkanRenderable("res/Models/ancientStatue.obj", {"res/Models/ancientStatue.png"}, {"res/Shaders/VulkanRayTracingShaders/callable.rcall"}, {0, 0, 0}, {0, 0, 0}, {1, 1, 1});
                renderEngine.uploadRenderable(&ancientStatue, true);
                double lastTab{0};
                double lastF2{0};
                double lastEsc{0};
                double lastCursorPosX{0};
                double lastCursorPosY{0};
                bool captureInput{};
                std::vector<float> recordedFPS{0};
                float recordedFPSCount{200};
                recordedFPS.resize((size_t)recordedFPSCount);
                while (renderEngine.update()) {
                    //Process inputs
                    glfwPollEvents();
                    float velocity = renderEngine.frameTime * renderEngine.settings.movementSpeed;
                    if ((bool)glfwGetKey(renderEngine.window, GLFW_KEY_F1)) {
                        for (VulkanRenderable *renderable : renderEngine.renderables) { renderable->reloadRenderable(); }
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
                                glfwSetCursorPosCallback(renderEngine.window, vulkanRayTracerCursorCallback);
                                glfwSetCursorPos(renderEngine.window, lastCursorPosX, lastCursorPosY);
                            } else {
                                glfwSetCursorPos(renderEngine.window, 0, 0);
                                glfwSetInputMode(renderEngine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                                glfwSetCursorPosCallback(renderEngine.window, vulkanRayTracerCursorCallback);
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
                    recordedFPS[(size_t)std::fmod((float)renderEngine.frameNumber, recordedFPSCount)] = 1 / renderEngine.frameTime;
                    int sum{0};
                    std::for_each(recordedFPS.begin(), recordedFPS.end(), [&] (int n) { sum += n; });
                    glfwSetWindowTitle(renderEngine.window, (std::string("Illumination Engine - ") + std::to_string((float)sum / recordedFPSCount)).c_str());
                }
                renderEngine.destroy();
            } catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
                return EXIT_FAILURE;
            }
            return EXIT_SUCCESS;
        }
    #endif
#endif
#ifdef ILLUMINATION_ENGINE_OPENGL
    if (input == "o") {
        try {
            OpenGLRenderEngine renderEngine = OpenGLRenderEngine();
            glfwSetWindowPosCallback(renderEngine.window, openglWindowPositionCallback);
            OpenGLRenderable vikingRoom = OpenGLRenderable("res/Models/vikingRoom.obj", {"res/Models/vikingRoom.png"}, {"res/Shaders/OpenGLShaders/vertexShader.glsl", "res/Shaders/OpenGLShaders/fragmentShader.glsl"});
            OpenGLRenderable cube = OpenGLRenderable("res/Models/cube.obj", {"res/Models/cube.png"}, {"res/Shaders/OpenGLShaders/vertexShader.glsl", "res/Shaders/OpenGLShaders/fragmentShader.glsl"});
            renderEngine.uploadRenderable(&vikingRoom);
            renderEngine.uploadRenderable(&cube);
            double lastTab{0};
            double lastF2{0};
            double lastEsc{0};
            double lastCursorPosX{0};
            double lastCursorPosY{0};
            bool captureInput{};
            std::vector<float> recordedFPS{};
            float recordedFPSCount{200};
            recordedFPS.resize((size_t)recordedFPSCount);
            renderEngine.camera.position = {0, 3, 0};
            while (renderEngine.update() != 1) {
                //Process inputs
                glfwPollEvents();
                float velocity = (float)renderEngine.frameTime * (float)renderEngine.settings.movementSpeed;
//                if ((bool)glfwGetKey(renderEngine.window, GLFW_KEY_F1)) {
//                    for (VulkanRenderable *renderable : renderEngine.renderables) { renderable->reloadRenderable(); }
//                    renderEngine.updateSettings(false);
                /*}*/ if ((bool)glfwGetKey(renderEngine.window, GLFW_KEY_F2) & (glfwGetTime() - lastF2 > .2)) {
                    renderEngine.settings.fullscreen = !renderEngine.settings.fullscreen;
                    renderEngine.updateSettings(true);
                    lastF2 = glfwGetTime();
                } if ((bool)glfwGetKey(renderEngine.window, GLFW_KEY_1)) {
                    renderEngine.settings.msaaSamples = 1;
                    renderEngine.updateSettings(true);
                } if ((bool)glfwGetKey(renderEngine.window, GLFW_KEY_4)) {
                    renderEngine.settings.msaaSamples = 4;
                    renderEngine.updateSettings(true);
                }
                if ((bool)glfwGetKey(renderEngine.window, GLFW_KEY_LEFT_CONTROL) & captureInput) { velocity *= 6; }
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
                            glfwSetCursorPosCallback(renderEngine.window, openglCursorCallback);
                            glfwSetCursorPos(renderEngine.window, lastCursorPosX, lastCursorPosY);
                        } else {
                            glfwSetCursorPos(renderEngine.window, 0, 0);
                            glfwSetInputMode(renderEngine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                            glfwSetCursorPosCallback(renderEngine.window, openglCursorCallback);
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
                recordedFPS[(size_t)std::fmod((float)renderEngine.frameNumber, recordedFPSCount)] = 1.0f / (float)renderEngine.frameTime;
                int sum{0};
                std::for_each(recordedFPS.begin(), recordedFPS.end(), [&] (int n) { sum += n; });
                glfwSetWindowTitle(renderEngine.window, (std::string("Illumination Engine - ") + std::to_string((float)sum / recordedFPSCount)).c_str());
            }
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }
#endif
}
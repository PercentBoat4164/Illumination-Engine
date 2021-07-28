#pragma once

#include <glm/gtc/matrix_transform.hpp>
#include <cmath>
#include <filesystem> //necessary for msvc

#ifndef ILLUMINATION_ENGINE_PI
#define ILLUMINATION_ENGINE_PI 3.141592653589793238462643383279
#endif

class VulkanCamera {
public:
    void create(VulkanGraphicsEngineLink *engineLink) {
        linkedRenderEngine = engineLink;
        aspectRatio = double(linkedRenderEngine->settings->resolution[0]) / linkedRenderEngine->settings->resolution[1];
        fov = linkedRenderEngine->settings->fov;
        proj = glm::perspective(glm::radians(linkedRenderEngine->settings->fov), aspectRatio, 0.01, linkedRenderEngine->settings->renderDistance);
        view = glm::lookAt(position, position + front, up);
    }

    void update() {
        right = glm::normalize(glm::cross(front, up));
        view = {glm::lookAt(position, position + front, up)};
        proj = {glm::perspective(glm::radians(horizontalFOV), double(linkedRenderEngine->settings->resolution[0]) / linkedRenderEngine->settings->resolution[1], 0.01, linkedRenderEngine->settings->renderDistance)};
        proj[1][1] *= -1.0f;
    }

    void updateSettings() {
        aspectRatio = double(linkedRenderEngine->settings->resolution[0]) / linkedRenderEngine->settings->resolution[1];
        fov = linkedRenderEngine->settings->fov;
        horizontalFOV = tanh(tan(linkedRenderEngine->settings->fov*(ILLUMINATION_ENGINE_PI/360)) * 1 / aspectRatio) * (360 / ILLUMINATION_ENGINE_PI);
    };

    VulkanGraphicsEngineLink *linkedRenderEngine{};
    glm::vec3 position{0, 0, 2};
    float yaw{-90};
    float pitch{0};
    double aspectRatio{16.0f/9};
    double fov{90};
    double horizontalFOV{tanh(tan(fov*(ILLUMINATION_ENGINE_PI/360)) * 1 / aspectRatio) * (360 / ILLUMINATION_ENGINE_PI)};
    glm::vec3 front{glm::normalize(glm::vec3{cos(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(pitch))})};
    glm::vec3 up{0, 0, 1};
    glm::vec3 right{glm::cross(front, up)};
    glm::mat4 view{};
    glm::mat4 proj{};
    std::deque<std::function<void()>> deletionQueue{};
};
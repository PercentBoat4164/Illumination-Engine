#pragma once

#include "vulkanUniformBufferObject.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <cmath>

#define ILLUMINATION_ENGINE_PI 3.141592653589793238462643383279

class VulkanCamera {
public:
    void create(VulkanGraphicsEngineLink *engineLink) {
        linkedRenderEngine = engineLink;
        updateSettings();
        projectionMatrix = glm::perspective(glm::radians(fov), aspectRatio, 0.01, linkedRenderEngine->settings->renderDistance);
    }

    void update() {
        front = glm::normalize(glm::vec3{cos(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(pitch))});
        right = glm::normalize(glm::cross(front, up));
        viewMatrix = {glm::lookAt(position, position + front, up)};
        projectionMatrix = {glm::perspective(glm::radians(horizontalFOV), double(linkedRenderEngine->settings->resolution[0]) / linkedRenderEngine->settings->resolution[1], 0.01, linkedRenderEngine->settings->renderDistance)};
        projectionMatrix[1][1] *= -1.0f;
    }

    void updateSettings() {
        aspectRatio = double(linkedRenderEngine->settings->resolution[0]) / linkedRenderEngine->settings->resolution[1];
        fov = linkedRenderEngine->settings->fov;
        horizontalFOV = tanh(tan(linkedRenderEngine->settings->fov*(ILLUMINATION_ENGINE_PI / 360)) * 1 / aspectRatio) * (360 / ILLUMINATION_ENGINE_PI);
    };

    VulkanGraphicsEngineLink *linkedRenderEngine{};
    float yaw{-90};
    float pitch{0};
    double aspectRatio{16.0/9};
    double fov{90};
    double horizontalFOV{tanh(tan(fov*(ILLUMINATION_ENGINE_PI / 360)) * 1 / aspectRatio) * (360 / ILLUMINATION_ENGINE_PI)};
    glm::vec3 front{glm::normalize(glm::vec3{cos(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(pitch))})};
    glm::vec3 up{0, 0, 1};
    glm::vec3 right{glm::cross(front, up)};
    glm::mat4 viewMatrix{glm::lookAt(position, position + front, up)};
    glm::mat4 projectionMatrix{};
    glm::vec3 position{};
    float speed{1};
};
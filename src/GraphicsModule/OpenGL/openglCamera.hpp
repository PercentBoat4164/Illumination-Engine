#pragma once

#include "openglSettings.hpp"
#include "openglGraphicsEngineLink.hpp"

#ifndef ILLUMINATION_ENGINE_PI
#define ILLUMINATION_ENGINE_PI 3.141592653589793238462643383279
#endif

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>

class OpenGLCamera {
public:
    void create(OpenGLGraphicsEngineLink *engineLink) {
        linkedRenderEngine = engineLink;
        updateSettings();
        proj = glm::perspective(glm::radians(horizontalFOV), aspectRatio, 0.01, linkedRenderEngine->settings->renderDistance);
    }

    void update() {
        front = glm::normalize(glm::vec3{cos(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(pitch))});
        right = glm::normalize(glm::cross(front, up));
        view = glm::lookAt(position, position + front, up);
        proj = glm::perspective(glm::radians(horizontalFOV), aspectRatio, 0.01, linkedRenderEngine->settings->renderDistance);
        proj[1][1] *= 1;
    }

    void updateSettings() {
        aspectRatio = double(linkedRenderEngine->settings->resolution[0]) / linkedRenderEngine->settings->resolution[1];
        fov = linkedRenderEngine->settings->fov;
        horizontalFOV = tanh(tan(linkedRenderEngine->settings->fov*(ILLUMINATION_ENGINE_PI/360)) * 1 / aspectRatio) * (360 / ILLUMINATION_ENGINE_PI);
    };

    OpenGLGraphicsEngineLink *linkedRenderEngine{};
    glm::vec3 position{0, 0, 0};
    glm::vec3 front{0, 1, 0};
    glm::vec3 up{0, 0, 1};
    glm::vec3 right{glm::cross(front, up)};
    float yaw{-90};
    float pitch{};
    double aspectRatio{1};
    double fov{90};
    double horizontalFOV{tanh(tan(fov*(ILLUMINATION_ENGINE_PI/360)) * 1 / aspectRatio) * (360 / ILLUMINATION_ENGINE_PI)};
    glm::mat4 view{glm::lookAt(position, position + front, up)};
    glm::mat4 proj{};
    float speed{1};
};
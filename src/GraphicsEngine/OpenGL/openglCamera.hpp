#pragma once

#include "openglSettings.hpp"

#ifndef ILLUMINATION_ENGINE_PI
#define ILLUMINATION_ENGINE_PI 3.141592653589793238462643383279
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>

class OpenGLCamera {
public:
    void create(OpenGLSettings *initialSettings) {
        updateSettings(initialSettings);
        proj = glm::perspective(glm::radians(horizontalFOV), aspectRatio, 0.01, settings->renderDistance);
    }

    glm::mat4 update() {
        front = glm::normalize(glm::vec3{cos(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(pitch))});
        right = glm::normalize(glm::cross(front, up));
        view = glm::lookAt(position, position + front, up);
        proj = glm::perspective(glm::radians(horizontalFOV), aspectRatio, 0.01, settings->renderDistance);
        proj[1][1] *= 1;
        return proj * view;
    }

    void updateSettings(OpenGLSettings *newSettings) {
        settings = newSettings;
        aspectRatio = double(settings->resolution[0]) / settings->resolution[1];
        fov = settings->fov;
        horizontalFOV = tanh(tan(settings->fov*(ILLUMINATION_ENGINE_PI/360)) * 1 / aspectRatio) * (360 / ILLUMINATION_ENGINE_PI);
    };

    OpenGLSettings *settings{};
    glm::vec3 position{0, 0, 0};
    glm::vec3 front{0, 1, 0};
    glm::vec3 up{0, 0, 1};
    glm::vec3 right{glm::cross(front, up)};
    float yaw{-90};
    float pitch{};
    double aspectRatio{16.0f/9};
    double fov{90};
    double horizontalFOV{tanh(tan(fov*(ILLUMINATION_ENGINE_PI/360)) * 1 / aspectRatio) * (360 / ILLUMINATION_ENGINE_PI)};
    glm::mat4 view{glm::lookAt(position, position + front, up)};
    glm::mat4 proj{};
};
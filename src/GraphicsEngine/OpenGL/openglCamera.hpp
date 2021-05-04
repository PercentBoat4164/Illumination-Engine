#pragma once

#include "openglSettings.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <array>

class OpenGLCamera {
public:
    explicit OpenGLCamera(OpenGLSettings *openglSettings) {
        settings = openglSettings;
        proj = glm::perspective(glm::radians(settings->fov), double(settings->resolution[0]) / settings->resolution[1], 0.01, settings->renderDistance);
    }

    glm::mat4 update() {
        front = glm::normalize(glm::vec3{cos(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(pitch))});
        right = glm::normalize(glm::cross(front, up));
        view = glm::lookAt(position, position + front, up);
        proj = glm::perspective(glm::radians(settings->fov), double(settings->resolution[0]) / settings->resolution[1], 0.01, settings->renderDistance);
        proj[1][1] *= 1;
        return proj * view;
    }

    OpenGLSettings *settings{};
    glm::vec3 position{0, 0, 2};
    glm::vec3 front{0, 1, 0};
    glm::vec3 up{0, 0, 1};
    glm::vec3 right{glm::cross(front, up)};
    float yaw{-90};
    float pitch{};
    glm::mat4 view{glm::lookAt(position, position + front, up)};
    glm::mat4 proj{};
};
#pragma once

#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "VulkanSettings.hpp"

class Camera {
public:
    std::array<glm::mat4, 2> update() {
        front = glm::normalize(glm::vec3{cos(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(pitch))});
        right = glm::normalize(glm::cross(front, up));
        view = {glm::lookAt(position, position + front, up)};
        proj = {glm::perspective(glm::radians(settings->fov), double(settings->resolution[0]) / std::max(settings->resolution[1], 1), 0.01, settings->renderDistance)};
        proj[1][1] *= -1;
        return {view, proj};
    }

    glm::vec3 position{0, 0, 2};
    glm::vec3 front{0, 1, 0};
    glm::vec3 up{0, 0, 1};
    glm::vec3 right{glm::cross(front, up)};
    float yaw{-90};
    float pitch{};
    glm::mat4 view{glm::lookAt(position, position + front, up)};
    glm::mat4 proj{glm::perspective(glm::radians(settings->fov), double(settings->resolution[0]) / std::max(settings->resolution[1], 1), 0.01, settings->renderDistance)};
    VulkanSettings *settings{};
};
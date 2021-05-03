#pragma once

#include <array>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "vulkanSettings.hpp"

/** This is the camera class.
 * It holds the methods that can setup and update the camera.*/
class Camera {
public:
    /** This method sets a few variables for the camera.
     * @param vulkanSettings Vulkan Settings is the file that holds many variables used in the camera class.*/
    explicit Camera(VulkanSettings *vulkanSettings) {
        settings = vulkanSettings;
        proj = glm::perspective(glm::radians(settings->fov), double(settings->resolution[0]) / std::max(settings->resolution[1], 1), 0.01, settings->renderDistance);
    }

    /** This method updates the camera view.
     * @return It returns the view that was generated.*/
    std::array<glm::mat4, 2> update() {
        front = glm::normalize(glm::vec3{cos(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(yaw)) * cos(glm::radians(pitch)), sin(glm::radians(pitch))});
        right = glm::normalize(glm::cross(front, up));
        view = {glm::lookAt(position, position + front, up)};
        proj = {glm::perspective(glm::radians(settings->fov), double(settings->resolution[0]) / std::max(settings->resolution[1], 1), 0.01, settings->renderDistance)};
        proj[1][1] *= -1;
        return {view, proj};
    }

    /** This is a local instance of VulkanSettings.*/
    VulkanSettings *settings{};
    /** This is a vector3 called position.*/
    glm::vec3 position{0, 0, 2};
    /** This is a vector3 called front.*/
    glm::vec3 front{0, 1, 0};
    /** This is a vector3 called up.*/
    glm::vec3 up{0, 0, 1};
    /** This is a vector3 called right.*/
    glm::vec3 right{glm::cross(front, up)};
    /** This variable holds the current yaw.*/
    float yaw{-90};
    /** This variable holds the current pitch.*/
    float pitch{};
    /** This is a matrix4 variable called view.*/
    glm::mat4 view{glm::lookAt(position, position + front, up)};
    /** This is a matrix4 variable called proj.*/
    glm::mat4 proj{};
};
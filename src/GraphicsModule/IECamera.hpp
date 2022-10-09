#pragma once
/* Define macros used throughout the file. */
#define ILLUMINATION_ENGINE_PI 3.141592653589793238462643383279

/* Predefine classes used with pointers or as return values for functions. */
class IERenderEngine;

/* Include classes used as attributes or function arguments. */

// Internal dependencies

// Modular dependencies

// External dependencies
#define GLM_FORCE_RADIANS

#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>

// System dependencies
#include <cmath>

class IECamera {
public:
    void create(IERenderEngine *engineLink);

    void update();

    void updateSettings();

    IERenderEngine *linkedRenderEngine{};
    float           yaw{90};
    float           pitch{0};
    double          aspectRatio{16.0 / 9};
    double          fov{90};
    double          horizontalFOV{
      tanh(tan(fov * (ILLUMINATION_ENGINE_PI / 360)) * 1 / aspectRatio) * (360 / ILLUMINATION_ENGINE_PI)};
    glm::vec3 front{glm::normalize(glm::vec3{
      cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
      sin(glm::radians(yaw)) * cos(glm::radians(pitch)),
      sin(glm::radians(pitch))})};
    glm::vec3 up{0, 0, 1};
    glm::vec3 right{glm::cross(front, up)};
    glm::mat4 viewMatrix{glm::lookAt(position, position + front, up)};
    glm::mat4 projectionMatrix{};
    glm::vec3 position{};
    float     speed{1};
};
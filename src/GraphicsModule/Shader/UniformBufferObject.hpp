#pragma once

/* Include classes used as attributes or function arguments. */
// External dependencies
#define GLM_FORCE_RADIANS

#include "glm/glm.hpp"

#include <string>

#define GLEW_IMPLEMENTATION
#include <GL/glew.h>

struct UniformBufferObject {
public:
    alignas(16) glm::mat4 projectionViewModelMatrix{};
    alignas(16) glm::mat4 modelMatrix{};
    alignas(16) glm::mat4 normalMatrix{};
    alignas(16) glm::vec3 position{0, 0, 0};
    alignas(4) glm::float32 time{};

    void openglUploadUniform(GLint program);
};
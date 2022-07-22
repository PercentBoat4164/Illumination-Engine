#pragma once

/* Include classes used as attributes or function arguments. */
// External dependencies
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include <string>

#define GLEW_IMPLEMENTATION  // Must precede GLEW inclusion.
#include "GL/glew.h"

struct IEUniformBufferObject {
public:
	alignas(16) glm::mat4 viewModelMatrix{};
	alignas(16) glm::mat4 modelMatrix{};
	alignas(16) glm::mat4 projectionMatrix{};
	alignas(16) glm::mat4 normalMatrix{};
	alignas(16) glm::vec3 position{0, 0, 0};
	alignas(4) glm::float32 time{};
	
	void openglUploadUniform(const std::string &name, GLint program);
};
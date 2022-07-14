#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class VkVertexInputBindingDescription;

class VkVertexInputAttributeDescription;

/* Include classes used as attributes or function arguments. */
// External dependencies

#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>

// System dependencies
#include <array>
#include <string>

struct IEVertex {
	glm::vec3 position{};
	glm::vec4 color{};
	glm::vec2 textureCoordinates{};
	glm::vec3 normal{};
	glm::vec3 tangent{};
	glm::vec3 biTangent{};

	static VkVertexInputBindingDescription getBindingDescription();

	static std::array<VkVertexInputAttributeDescription, 6> getAttributeDescriptions();

	bool operator==(IEVertex &other);
};
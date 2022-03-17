#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class VkVertexInputBindingDescription;

class VkVertexInputAttributeDescription;

/* Include classes used as attributes or function arguments. */
// External dependencies
#include <glm/glm.hpp>

// System dependencies
#include <array>

struct IEVertex {
    glm::vec3 position{};
    glm::vec4 color{};
    glm::vec2 textureCoordinates{};
    glm::vec3 normal{};

    static VkVertexInputBindingDescription getBindingDescription();

    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions();
};
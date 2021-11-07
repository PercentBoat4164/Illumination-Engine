#pragma once

#include <glm/glm.hpp>

struct VulkanUniformBufferObject {
public:
    alignas(16) glm::mat4 viewModelMatrix{};
    alignas(16) glm::mat4 modelMatrix{};
    alignas(16) glm::mat4 projectionMatrix{};
    alignas(16) glm::mat4 normalMatrix{};
    alignas(16) glm::vec3 position{0, 0, 0};
    alignas(4) glm::float32 time{};
};
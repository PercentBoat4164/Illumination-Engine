#pragma once

#include <glm/glm.hpp>

struct VulkanUniformBufferObject {
public:
    alignas(16) glm::mat4 model{};
    alignas(16) glm::mat4 view{};
    alignas(16) glm::mat4 proj{};
    alignas(16) float time{};
};
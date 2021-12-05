#ifdef ILLUMINATION_ENGINE_VULKAN
#include <vulkan/vulkan.hpp>
#endif

#include "glm/glm.hpp"

#include <array>

class IeVertex {
    glm::vec3 position{};
    glm::vec2 textureCoordinates{};
    glm::vec3 normal{};

    #ifdef ILLUMINATION_ENGINE_VULKAN
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(IeVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(IeVertex, position);
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 2;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(IeVertex, textureCoordinates);
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 3;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(IeVertex, normal);
        return attributeDescriptions;
    }
    #endif
};
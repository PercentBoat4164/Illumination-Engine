#pragma once

#include <glew/include/GL/glew.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

#include <iostream>
#include <cstring>
#include <fstream>
#include <cstdlib>
#include <set>
#include <deque>
#include <functional>
#include <filesystem>
#include <optional>

#if defined(_WIN32)
#define GLSLC "glslc.exe "
#else
#define GLSLC "glslc "
#endif

struct RenderEngineLink {
    Settings settings{};
    VkDevice logicalDevice{};
    VkPhysicalDevice physicalDevice{};
    VkDescriptorSetLayout descriptorSetLayout{};
    std::vector<VkDescriptorSet> descriptorSets{};
    std::vector<VkBuffer> uniformBuffers{};
    VkCommandPool commandPool{};
    VkQueue graphicsQueue{};

    [[nodiscard]] VkCommandBuffer beginSingleTimeCommands() const {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = 1;
        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer);
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        return commandBuffer;
    }

    void endSingleTimeCommands(VkCommandBuffer commandBuffer) const {
        vkEndCommandBuffer(commandBuffer);
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(graphicsQueue);
        vkFreeCommandBuffers(logicalDevice, commandPool, 1, &commandBuffer);
    }
};

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct AllocatedBuffer {
    VkBuffer buffer{};
    VkDeviceMemory memory{};
    RenderEngineLink linkedRenderEngine{};
    std::deque<std::function<void()>> deletionQueue{};

    ~AllocatedBuffer() {
        for (std::function<void()>& function : deletionQueue) {function();}
        deletionQueue.clear();
    }

    void setEngineLink(const RenderEngineLink& renderEngineLink) {
        linkedRenderEngine = renderEngineLink;
    }

    void *create(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if (vkCreateBuffer(linkedRenderEngine.logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {throw std::runtime_error("failed to create buffer!");}
        VkMemoryRequirements memoryRequirements;
        vkGetBufferMemoryRequirements(linkedRenderEngine.logicalDevice, buffer, &memoryRequirements);
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memoryRequirements.size;
        VkPhysicalDeviceMemoryProperties memoryProperties{};
        vkGetPhysicalDeviceMemoryProperties(linkedRenderEngine.physicalDevice, &memoryProperties);
        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {if ((memoryRequirements.memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) { allocInfo.memoryTypeIndex = i; break;}}
        if (vkAllocateMemory(linkedRenderEngine.logicalDevice, &allocInfo, nullptr, &memory) != VK_SUCCESS) {throw std::runtime_error("failed to create buffer memory!");}
        vkBindBufferMemory(linkedRenderEngine.logicalDevice, buffer, memory, 0);
        void *data;
        vkMapMemory(linkedRenderEngine.logicalDevice, memory, 0, size, 0, &data);
        deletionQueue.emplace_front([&]{vkDestroyBuffer(linkedRenderEngine.logicalDevice, buffer, nullptr);});
        deletionQueue.emplace_front([&]{vkFreeMemory(linkedRenderEngine.logicalDevice, memory, nullptr);});
        deletionQueue.emplace_front([&]{vkUnmapMemory(linkedRenderEngine.logicalDevice, memory);});
        return data;
    };

    void toImage(VkImage image, uint32_t width, uint32_t height) const {
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, 1};
        VkCommandBuffer commandBuffer = linkedRenderEngine.beginSingleTimeCommands();
        vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
        linkedRenderEngine.endSingleTimeCommands(commandBuffer);
    }
};

struct AllocatedImage {
    VkImage image{};
    VkDeviceMemory memory{};
    RenderEngineLink linkedRenderEngine{};
    std::deque<std::function<void()>> deletionQueue{};

    ~AllocatedImage() {
        for (std::function<void()>& function : deletionQueue) {function();}
        deletionQueue.clear();
    }

    void setEngineLink(const RenderEngineLink& renderEngineLink) {
        linkedRenderEngine = renderEngineLink;
    }

    void create(int width, int height, int mipLevels, VkFormat format, VkImageTiling tiling, int usage, VkMemoryPropertyFlagBits properties,VkSampleCountFlagBits msaaSamples) {
        VkImageCreateInfo imageCreateInfo{};
        imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        imageCreateInfo.extent.width = width;
        imageCreateInfo.extent.height = height;
        imageCreateInfo.extent.depth = 1;
        imageCreateInfo.mipLevels = mipLevels;
        imageCreateInfo.arrayLayers = 1;
        imageCreateInfo.format = format;
        imageCreateInfo.tiling = tiling;
        imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageCreateInfo.usage = usage;
        imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageCreateInfo.samples = msaaSamples;
        if (vkCreateImage(linkedRenderEngine.logicalDevice, &imageCreateInfo, nullptr, &image) != VK_SUCCESS) {throw std::runtime_error("failed to create image!");}
        VkMemoryRequirements memoryRequirements{};
        vkGetImageMemoryRequirements(linkedRenderEngine.logicalDevice, image, &memoryRequirements);
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memoryRequirements.size;
        VkPhysicalDeviceMemoryProperties memoryProperties{};
        vkGetPhysicalDeviceMemoryProperties(linkedRenderEngine.physicalDevice, &memoryProperties);
        for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {if ((memoryRequirements.memoryTypeBits & (1 << i)) && (memoryProperties.memoryTypes[i].propertyFlags & properties) == properties) { allocInfo.memoryTypeIndex = i; break;}}
        if (vkAllocateMemory(linkedRenderEngine.logicalDevice, &allocInfo, nullptr, &memory) != VK_SUCCESS) {throw std::runtime_error("failed to create image memory!");}
        vkBindImageMemory(linkedRenderEngine.logicalDevice, image, memory, 0);
        deletionQueue.emplace_front([&]{vkDestroyImage(linkedRenderEngine.logicalDevice, image, nullptr);});
        deletionQueue.emplace_front([&]{vkFreeMemory(linkedRenderEngine.logicalDevice, memory, nullptr);});
    }

    void transition(VkImageLayout oldLayout, VkImageLayout newLayout) const {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        VkPipelineStageFlags sourceStage{};
        VkPipelineStageFlags destinationStage{};
        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {throw std::invalid_argument("unsupported layout transition!");}
        VkCommandBuffer commandBuffer = linkedRenderEngine.beginSingleTimeCommands();
        vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
        linkedRenderEngine.endSingleTimeCommands(commandBuffer);
    }
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec3 normal;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription;
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }

    bool operator==(const Vertex &other) const {return pos == other.pos && color == other.color && texCoord == other.texCoord;}
};

namespace std {template<> struct hash<Vertex> {size_t operator()(Vertex const& vertex) const {return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.normal) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);}};}

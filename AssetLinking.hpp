#pragma once

#include <glew/include/GL/glew.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

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

#if defined(_WIN32)
#define GLSLC "glslc.exe "
#else
#define GLSLC "glslc "
#endif

struct RenderEngineLink {
    VmaAllocator allocator{};
    Settings settings{};
    std::deque<std::function<void()>> deletionQueue{};
    VkDevice logicalDevice{};
    VkPhysicalDevice physicalDevice{};
    std::vector<VkImage> swapChainImages{};
    VkDescriptorSetLayout descriptorSetLayout{};
    VkDescriptorPool descriptorPool{};
    std::vector<VkDescriptorSet> descriptorSets{};
    std::vector<VkBuffer> uniformBuffers{};
    VkCommandPool commandPool{};
    VkQueue singleTimeQueue{};
};

struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

struct AllocatedBuffer {
    VkBuffer buffer{};
    VmaAllocation allocation{};
    VmaAllocator allocator{};
    VkDeviceMemory memory{};
    VkDevice device{};

    void allocate(VkDeviceSize size, VkBufferUsageFlagBits bufferUsage, VkSharingMode sharingMode, VmaMemoryUsage memoryUsage) {
        if (device == VK_NULL_HANDLE) {throw std::runtime_error("initialize device before allocating!");}
        VkBufferCreateInfo bufferCreateInfo{};
        bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferCreateInfo.size = size;
        bufferCreateInfo.usage = bufferUsage;
        bufferCreateInfo.sharingMode = sharingMode;
        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = memoryUsage;
        vmaCreateBuffer(allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, nullptr);
        vkBindBufferMemory(device, buffer, memory, 0);
    }

    ~AllocatedBuffer() {vmaDestroyBuffer(allocator, buffer, allocation);}
};

struct AllocatedImage {
    VkImage image{};
    VmaAllocation allocation{};
    VmaAllocator allocator{};
    VkDeviceMemory memory{};
    VkDevice device{};

    void allocate(int width, int height, int mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlagBits usage, VkSampleCountFlagBits msaaSamples, VmaMemoryUsage memoryUsage) {
        if (image == VK_NULL_HANDLE) {throw std::runtime_error("initialize device before allocating!");}
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
        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = memoryUsage;
        vmaCreateImage(allocator, &imageCreateInfo, &allocationCreateInfo, &image, &allocation, nullptr);
        vkBindImageMemory(device, image, memory, 0);
    }

    ~AllocatedImage() {vmaDestroyImage(allocator, image, allocation);}
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

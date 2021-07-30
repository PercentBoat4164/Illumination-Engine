#pragma once

#include "vulkanGraphicsEngineLink.hpp"

#include <deque>
#include <functional>

class VulkanImage;

class VulkanBuffer {
public:
    struct CreateInfo {
        //Only required for buffer
        VkDeviceSize size{};
        VkBufferUsageFlags usage{};
        VmaMemoryUsage allocationUsage{};

        //Only required for acceleration structure creation
        VkAccelerationStructureTypeKHR type{};
        VkTransformMatrixKHR *transformationMatrix{};
        uint32_t primitiveCount{1};

        //Optional, only available for acceleration structures
        VkAccelerationStructureKHR accelerationStructureToModify{};

        //Only required if type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR
        VkDeviceAddress vertexBufferAddress{};
        VkDeviceAddress indexBufferAddress{};
        VkDeviceAddress transformationBufferAddress{};

        //Only required if type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR
        std::vector<VkDeviceAddress> bottomLevelAccelerationStructureDeviceAddresses{};

        //Optional
        void *data{};

        //Required only if data != nullptr
        uint32_t sizeOfData{};
    };

    void *data{};
    VkBuffer buffer{};
    VkDeviceAddress deviceAddress{};
    CreateInfo createdWith{};
    bool created{false};

    void destroy() {
        if (!created) {
            throw std::runtime_error("Calling VulkanBuffer::destroy() on a buffer for which VulkanBuffer::create() has not been called is illegal.");
        }
        for (std::function<void()> &function : deletionQueue) { function(); }
        deletionQueue.clear();
        created = false;
    }

    virtual void create(VulkanGraphicsEngineLink *engineLink, CreateInfo *createInfo) {
        linkedRenderEngine = engineLink;
        createdWith = *createInfo;
        VkBufferCreateInfo bufferCreateInfo{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
        bufferCreateInfo.size = createdWith.size;
        bufferCreateInfo.usage = createdWith.usage;
        VmaAllocationCreateInfo allocationCreateInfo{};
        allocationCreateInfo.usage = createdWith.allocationUsage;
        if (vmaCreateBuffer(*linkedRenderEngine->allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, nullptr) != VK_SUCCESS) { throw std::runtime_error("failed to create buffer!"); }
        deletionQueue.emplace_front([&] { vmaDestroyBuffer(*linkedRenderEngine->allocator, buffer, allocation); });
        if (createdWith.data != nullptr) {
            if (createdWith.sizeOfData > createdWith.size) { throw std::runtime_error("VulkanBuffer::CreateInfo::sizeOfData must not be greater than VulkanBuffer::CreateInfo::size."); }
            vmaMapMemory(*linkedRenderEngine->allocator, allocation, &data);
            memcpy(data, createdWith.data, createdWith.sizeOfData);
            vmaUnmapMemory(*linkedRenderEngine->allocator, allocation);
        }
        if (createdWith.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
            VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO};
            bufferDeviceAddressInfo.buffer = buffer;
            deviceAddress = linkedRenderEngine->vkGetBufferDeviceAddressKHR(linkedRenderEngine->device->device, &bufferDeviceAddressInfo);
        }
        created = true;
    }

    void uploadData(void *input, uint32_t sizeOfInput) {
        if (!created) { throw std::runtime_error("Calling VulkanBuffer::uploadData() on a buffer for which VulkanBuffer::create() has not been called is illegal."); }
        if (sizeOfInput > createdWith.size) { throw std::runtime_error("sizeOfInput must not be greater than VulkanBuffer::CreateInfo::size."); }
        vmaMapMemory(*linkedRenderEngine->allocator, allocation, &data);
        memcpy(data, input, sizeOfInput);
        vmaUnmapMemory(*linkedRenderEngine->allocator, allocation);
    }

    void toImage(const VulkanImage &image, uint32_t width, uint32_t height, VkCommandBuffer commandBuffer = nullptr) const;

protected:
    std::deque<std::function<void()>> deletionQueue{};
    VulkanGraphicsEngineLink *linkedRenderEngine{};
    VmaAllocation allocation{};
};

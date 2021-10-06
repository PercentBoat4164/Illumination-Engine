#pragma once

class Buffer {
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
    RenderEngineLink *linkedRenderEngine{};
    VmaAllocation allocation{};
    CreateInfo createdWith{};
    bool created{false};

    virtual void create(RenderEngineLink *engineLink, CreateInfo *createInfo) {
        linkedRenderEngine = engineLink;
        createdWith = *createInfo;
        VkBufferCreateInfo bufferCreateInfo{.sType=VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size=createdWith.size, .usage=createdWith.usage};
        VmaAllocationCreateInfo allocationCreateInfo{.usage=createdWith.allocationUsage};
        if (vmaCreateBuffer(linkedRenderEngine->allocator, &bufferCreateInfo, &allocationCreateInfo, &buffer, &allocation, nullptr) != VK_SUCCESS) { linkedRenderEngine->log->log("failed to create buffer!", log4cplus::DEBUG_LOG_LEVEL, "Graphics Module"); }
        created = true;
        if (createdWith.data != nullptr) {
            if (createdWith.sizeOfData > createdWith.size) { throw std::runtime_error("VulkanBuffer::CreateInfo::sizeOfData must not be greater than VulkanBuffer::CreateInfo::size."); }
            vmaMapMemory(linkedRenderEngine->allocator, allocation, &data);
            memcpy(data, createdWith.data, createdWith.sizeOfData);
            vmaUnmapMemory(linkedRenderEngine->allocator, allocation);
        }
        if (createdWith.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
            VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{.sType=VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer=buffer};
            deviceAddress = linkedRenderEngine->vkGetBufferDeviceAddressKHR(linkedRenderEngine->device.device, &bufferDeviceAddressInfo);
        }
    }

    virtual void destroy() {
        if (created) { vmaDestroyBuffer(linkedRenderEngine->allocator, buffer, allocation); }
    }
};
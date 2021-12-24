#pragma once

#include <variant>

class IEImage;

#ifdef ILLUMINATION_ENGINE_VULKAN
#include <vulkan/vulkan.hpp>
#endif

/**@todo: Implement something similar to what was done for the IEImage class for selection of properties.*/

class IEBuffer {
public:
    struct CreateInfo {
        //Only required for VulkanBuffer
        uint32_t bufferSize{};
        uint32_t usage{};
        #ifdef ILLUMINATION_ENGINE_OPENGL
        uint32_t target{};
        #endif
        #ifdef ILLUMINATION_ENGINE_VULKAN
        VmaMemoryUsage allocationUsage{};

        //Only required for acceleration structure creation
        uint32_t primitiveCount{1};
        VkAccelerationStructureTypeKHR type{};
        VkTransformMatrixKHR *transformationMatrix{};

        //Optional, only available for acceleration structures
        VkAccelerationStructureKHR accelerationStructureToModify{};

        //Only required if type == VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR
        VkDeviceAddress vertexBufferAddress{};
        VkDeviceAddress indexBufferAddress{};
        VkDeviceAddress transformationBufferAddress{};

        //Only required if type == VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR
        std::vector<VkDeviceAddress> bottomLevelAccelerationStructureDeviceAddresses{};
        #endif

        //Optional
        void *data{};

        //Required only if bufferData != nullptr
        uint32_t sizeOfData{};
    };

    struct Created {
        bool buffer{};
    };

    void *bufferData{};
    #ifdef ILLUMINATION_ENGINE_VULKAN
    VkBuffer VulkanBuffer{};
    VkDeviceAddress deviceAddress{};
    VmaAllocation allocation{};
    #endif
    uint32_t OpenGLBuffer{};
    IERenderEngineLink *linkedRenderEngine{};
    CreateInfo createdWith{};
    Created created{};

    virtual IEBuffer create(IERenderEngineLink *engineLink, CreateInfo *createInfo) {
        linkedRenderEngine = engineLink;
        createdWith = *createInfo;
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (linkedRenderEngine->api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
            VkBufferCreateInfo bufferCreateInfo{.sType=VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size=createdWith.bufferSize, .usage=createdWith.usage};
            VmaAllocationCreateInfo allocationCreateInfo{.usage=createdWith.allocationUsage};
            if (vmaCreateBuffer(linkedRenderEngine->allocator, &bufferCreateInfo, &allocationCreateInfo, &VulkanBuffer, &allocation, nullptr) != VK_SUCCESS) {
                IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Failed to create VulkanBuffer!");
            }
            created.buffer = true;
            if (createdWith.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
                VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{.sType=VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer=VulkanBuffer};
                deviceAddress = linkedRenderEngine->vkGetBufferDeviceAddressKHR(linkedRenderEngine->device.device, &bufferDeviceAddressInfo);
            }
        }
        #endif
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (linkedRenderEngine->api.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
            glGenBuffers(1, &OpenGLBuffer);
            created.buffer = true;
        }
        #endif
        if (createdWith.data != nullptr) { upload(createdWith.data, createdWith.sizeOfData); }
        return *this;
    }

    virtual IEBuffer upload(void *data, uint32_t sizeOfData) {
        if (!created.buffer) {
            IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Attempted to upload to a VulkanBuffer that does not exist!");
            return IEBuffer{};
        }
        if (sizeOfData > createdWith.bufferSize) {
            IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "sizeOfData must not be greater than bufferSize");
            return IEBuffer{};
        }
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (linkedRenderEngine->api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
            vmaMapMemory(linkedRenderEngine->allocator, allocation, &bufferData);
            memcpy(bufferData, data, sizeOfData);
            vmaUnmapMemory(linkedRenderEngine->allocator, allocation);
        }
        #endif
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (linkedRenderEngine->api.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
            glBindBuffer(createdWith.target, OpenGLBuffer);
            glBufferData(createdWith.target, sizeOfData, data, createdWith.usage);
        }
        #endif
        return *this;
    }

    #ifdef ILLUMINATION_ENGINE_VULKAN
    virtual void update(void *data, uint32_t sizeOfData, uint32_t startingPosition, VkCommandBuffer commandBuffer) {
        if (!created.buffer) {
            IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Attempted to update a VulkanBuffer that does not exist!");
            return;
        }
        if (data == nullptr) {
            IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Attempted to update an existing VulkanBuffer with no data!");
            return;
        }
        vkCmdUpdateBuffer(commandBuffer, VulkanBuffer, startingPosition, sizeOfData, data);
    }
    #endif

    void* getBuffer() {
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (linkedRenderEngine->api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
            return &VulkanBuffer;
        }
        #endif
        return &OpenGLBuffer;
    }

    #ifdef ILLUMINATION_ENGINE_VULKAN
    virtual void toImage(IEImage* image, uint16_t width, uint16_t height, VkCommandBuffer commandBuffer);
    #endif

    void destroy() {
        if (created.buffer) {
            #ifdef ILLUMINATION_ENGINE_VULKAN
            if (linkedRenderEngine->api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
                vmaDestroyBuffer(linkedRenderEngine->allocator, VulkanBuffer, allocation);
                created.buffer = false;
            }
            #endif
            #ifdef ILLUMINATION_ENGINE_OPENGL
            if (linkedRenderEngine->api.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
                glDeleteBuffers(1, &OpenGLBuffer);
                created.buffer = false;
            }
            #endif
        }
    }

    ~IEBuffer() {
        destroy();
    }
};
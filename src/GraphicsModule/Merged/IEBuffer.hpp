#pragma once

#include <variant>

class IEImage;

#ifndef ILLUMINATION_ENGINE_VULKAN
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkBuffer);
#endif

/**@todo: Implement something similar to what was done for the IEImage class for selection of properties.*/

class IEBuffer {
public:
    struct CreateInfo {
        //Only required for buffer
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
    std::variant<VkBuffer, uint32_t> buffer{};
    #ifdef ILLUMINATION_ENGINE_VULKAN
    VkDeviceAddress deviceAddress{};
    VmaAllocation allocation{};
    #endif
    IERenderEngineLink *linkedRenderEngine{};
    CreateInfo createdWith{};
    Created created{};

    virtual IEBuffer create(IERenderEngineLink *engineLink, CreateInfo *createInfo) {
        linkedRenderEngine = engineLink;
        createdWith = *createInfo;
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (linkedRenderEngine->api.name == "Vulkan") {
            VkBufferCreateInfo bufferCreateInfo{.sType=VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size=createdWith.bufferSize, .usage=createdWith.usage};
            VmaAllocationCreateInfo allocationCreateInfo{.usage=createdWith.allocationUsage};
            if (vmaCreateBuffer(linkedRenderEngine->allocator, &bufferCreateInfo, &allocationCreateInfo, &std::get<VkBuffer>(buffer), &allocation, nullptr) != VK_SUCCESS) { linkedRenderEngine->log->log("failed to create buffer!", log4cplus::DEBUG_LOG_LEVEL, "Graphics Module"); }
            created.buffer = true;
            if (createdWith.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
                VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{.sType=VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer=std::get<VkBuffer>(buffer)};
                deviceAddress = linkedRenderEngine->vkGetBufferDeviceAddressKHR(linkedRenderEngine->device.device, &bufferDeviceAddressInfo);
            }
        }
        #endif
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (linkedRenderEngine->api.name == "OpenGL") {
            glGenBuffers(1, &std::get<uint32_t>(buffer));
            created.buffer = true;
        }
        #endif
        if (createdWith.data != nullptr) { upload(createdWith.data, createdWith.sizeOfData); }
        return *this;
    }

    virtual IEBuffer upload(void *data, uint32_t sizeOfData) {
        if (!created.buffer) { linkedRenderEngine->log->log("Called IEBuffer::upload() on a IEBuffer that does not exist!", log4cplus::WARN_LOG_LEVEL, "Graphics Module"); }
        if (sizeOfData > createdWith.bufferSize) { linkedRenderEngine->log->log("IEBuffer::CreateInfo::sizeOfData must not be greater than IEBuffer::CreateInfo::bufferSize.", log4cplus::WARN_LOG_LEVEL, "Graphics Module"); }
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (linkedRenderEngine->api.name == "Vulkan") {
            vmaMapMemory(linkedRenderEngine->allocator, allocation, &bufferData);
            memcpy(bufferData, data, sizeOfData);
            vmaUnmapMemory(linkedRenderEngine->allocator, allocation);
        }
        #endif
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (linkedRenderEngine->api.name == "OpenGL") {
            glBindBuffer(createdWith.target, std::get<uint32_t>(buffer));
            glBufferData(createdWith.target, sizeOfData, data, createdWith.usage);
        }
        #endif
        return *this;
    }

    virtual void update(void *data, uint32_t sizeOfData, uint32_t startingPosition, VkCommandBuffer commandBuffer) {
        if (!created.buffer) {
            linkedRenderEngine->log->log("Attempted to update a buffer that has not been created!", log4cplus::ERROR_LOG_LEVEL, "Graphics Module");
        }
        if (data == nullptr) {
            linkedRenderEngine->log->log("Attempted to update a buffer with contentsString as nullptr!", log4cplus::WARN_LOG_LEVEL, "Graphics Module");
        }
        vkCmdUpdateBuffer(commandBuffer, std::get<VkBuffer>(buffer), startingPosition, sizeOfData, data);
    }

    virtual void toImage(IEImage* image, uint16_t width, uint16_t height, VkCommandBuffer commandBuffer);

    void destroy() {
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (linkedRenderEngine->api.name == "Vulkan") { if (created.buffer) { vmaDestroyBuffer(linkedRenderEngine->allocator, std::get<VkBuffer>(buffer), allocation); } }
        #endif
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (linkedRenderEngine->api.name == "OpenGL") { if (created.buffer) { glDeleteBuffers(1, &std::get<uint32_t>(buffer)); } }
        #endif
    }

    ~IEBuffer() {
        destroy();
    }
};
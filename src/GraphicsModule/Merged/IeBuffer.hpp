#pragma once

#include <variant>

class IeImage;

#ifndef ILLUMINATION_ENGINE_VULKAN
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkBuffer);
#endif

/**@todo: Implement something similar to what was done for the IeImage class for selection of properties.*/

class IeBuffer {
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

    void *bufferData{};
    std::variant<VkBuffer, uint32_t> buffer{};
    #ifdef ILLUMINATION_ENGINE_VULKAN
    VkDeviceAddress deviceAddress{};
    VmaAllocation allocation{};
    #endif
    IeRenderEngineLink *linkedRenderEngine{};
    CreateInfo createdWith{};
    bool created{false};

    virtual IeBuffer create(IeRenderEngineLink *engineLink, CreateInfo *createInfo) {
        linkedRenderEngine = engineLink;
        createdWith = *createInfo;
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (linkedRenderEngine->api.name == "Vulkan") {
            VkBufferCreateInfo bufferCreateInfo{.sType=VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO, .size=createdWith.bufferSize, .usage=createdWith.usage};
            VmaAllocationCreateInfo allocationCreateInfo{.usage=createdWith.allocationUsage};
            if (vmaCreateBuffer(linkedRenderEngine->allocator, &bufferCreateInfo, &allocationCreateInfo, &std::get<VkBuffer>(buffer), &allocation, nullptr) != VK_SUCCESS) { linkedRenderEngine->log->log("failed to create buffer!", log4cplus::DEBUG_LOG_LEVEL, "Graphics Module"); }
            created = true;
            if (createdWith.usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
                VkBufferDeviceAddressInfoKHR bufferDeviceAddressInfo{.sType=VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO, .buffer=std::get<VkBuffer>(buffer)};
                deviceAddress = linkedRenderEngine->vkGetBufferDeviceAddressKHR(linkedRenderEngine->device.device, &bufferDeviceAddressInfo);
            }
        }
        #endif
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (linkedRenderEngine->api.name == "OpenGL") {
            glGenBuffers(1, &std::get<uint32_t>(buffer));
            created = true;
        }
        #endif
        if (createdWith.data != nullptr) { upload(createdWith.data, createdWith.sizeOfData); }
        return *this;
    }

    virtual IeBuffer upload(void *data, uint32_t sizeOfData) {
        if (!created) { linkedRenderEngine->log->log("Called IeBuffer::upload() on a IeBuffer that does not exist!", log4cplus::WARN_LOG_LEVEL, "Graphics Module"); }
        if (sizeOfData > createdWith.bufferSize) { linkedRenderEngine->log->log("IeBuffer::CreateInfo::sizeOfData must not be greater than IeBuffer::CreateInfo::bufferSize.", log4cplus::WARN_LOG_LEVEL, "Graphics Module"); }
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

    virtual IeImage toImage(IeImage* image, uint32_t width, uint32_t height, VkCommandBuffer commandBuffer);

    virtual void destroy() {
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (linkedRenderEngine->api.name == "Vulkan") { if (created) { vmaDestroyBuffer(linkedRenderEngine->allocator, std::get<VkBuffer>(buffer), allocation); } }
        #endif
        #ifdef ILLUMINATION_ENGINE_OPENGL
        if (linkedRenderEngine->api.name == "OpenGL") { if (created) { glDeleteBuffers(1, &std::get<uint32_t>(buffer)); } }
        #endif
    }
};
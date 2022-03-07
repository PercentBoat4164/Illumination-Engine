#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IEImage;

class IERenderEngine;

/* Include classes used as attributes or function arguments. */
// External dependencies
#include "vk_mem_alloc.h"

#include <vulkan/vulkan.h>

// System dependencies
#include <vector>
#include <cstdint>
#include <functional>


class IEBuffer {
public:
    struct CreateInfo {
        //Only required for IEBuffer
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

        //Required only if bufferData != nullptr
        uint32_t sizeOfData{};
    };

    void *data{};
    VkBuffer buffer{};
    VkDeviceAddress deviceAddress{};
    CreateInfo createdWith{};
    bool created{false};

    void destroy();

    virtual void create(IERenderEngine *engineLink, CreateInfo *createInfo);

    void uploadData(void *input, uint32_t sizeOfInput);

    void toImage(IEImage *image, uint32_t width, uint32_t height);

    void toImage(IEImage *image);

    virtual ~IEBuffer();

protected:
    std::vector<std::function<void()>> deletionQueue{};
    IERenderEngine* linkedRenderEngine{};
    VmaAllocation allocation{};
};

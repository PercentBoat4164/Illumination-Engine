#pragma once

#include "vulkanSettings.hpp"

#include <VkBootstrap.h>

#ifndef VMA_IMPLEMENTATION
#define VMA_IMPLEMENTATION
#include "../../../deps/vk_mem_alloc.h"
#endif

#include <vector>

class VulkanGraphicsEngineLink {
public:
    struct PhysicalDeviceInfo {
        //Device Properties
        VkPhysicalDeviceProperties physicalDeviceProperties{};

        // Extension Properties
        VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2};
        void *pNextHighestProperty = &physicalDeviceMemoryProperties;

        // Device Features
        VkPhysicalDeviceFeatures physicalDeviceFeatures{};

        // Extension Features
        // NOTE: Ray tracing features are on the bottom of the pNext stack so that a pointer to higher up on the stack can grab only the structures supported by RenderDoc.
        VkPhysicalDeviceDescriptorIndexingFeaturesEXT physicalDeviceDescriptorIndexingFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT};
        // All the below are ray tracing features, and cannot be loaded by RenderDoc.
        VkPhysicalDeviceAccelerationStructureFeaturesKHR physicalDeviceAccelerationStructureFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR, &physicalDeviceDescriptorIndexingFeatures};
        VkPhysicalDeviceBufferDeviceAddressFeaturesEXT physicalDeviceBufferDeviceAddressFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES, &physicalDeviceAccelerationStructureFeatures};
        VkPhysicalDeviceRayQueryFeaturesKHR physicalDeviceRayQueryFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_QUERY_FEATURES_KHR, &physicalDeviceBufferDeviceAddressFeatures};
        VkPhysicalDeviceRayTracingPipelineFeaturesKHR physicalDeviceRayTracingPipelineFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR, &physicalDeviceRayQueryFeatures};
        void *pNextHighestFeature = &physicalDeviceRayTracingPipelineFeatures;
        void *pNextHighestRenderDocCompatibleFeature = &physicalDeviceDescriptorIndexingFeatures;

        //Engine Features
        VkBool32 anisotropicFiltering{false};
        VkBool32 msaaSmoothing{false};
        VkBool32 rayTracing{false};
    };

    VulkanSettings *settings = nullptr;
    vkb::Device *device{};
    vkb::Swapchain *swapchain{};
    VkCommandPool *commandPool{};
    VmaAllocator *allocator{};
    VkQueue *graphicsQueue{};
    VkQueue *presentQueue{};
    VkQueue *transferQueue{};
    VkQueue *computeQueue{};
    std::vector<VkImageView> *swapchainImageViews{};
    PFN_vkGetBufferDeviceAddress vkGetBufferDeviceAddressKHR{};
    PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR{};
    PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR{};
    PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR{};
    PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR{};
    PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR{};
    PFN_vkAcquireNextImageKHR vkAcquireNextImageKhr{};
    PhysicalDeviceInfo supportedPhysicalDeviceInfo{};
    PhysicalDeviceInfo enabledPhysicalDeviceInfo{};

    void build(bool renderDocCapturing = false) {
        vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(device->device, "vkGetBufferDeviceAddressKHR"));
        vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device->device, "vkCmdBuildAccelerationStructuresKHR"));
        vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(device->device, "vkCreateAccelerationStructureKHR"));
        vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(device->device, "vkDestroyAccelerationStructureKHR"));
        vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(device->device, "vkGetAccelerationStructureBuildSizesKHR"));
        vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(device->device, "vkGetAccelerationStructureDeviceAddressKHR"));
        vkAcquireNextImageKhr = reinterpret_cast<PFN_vkAcquireNextImageKHR>(vkGetDeviceProcAddr(device->device, "vkAcquireNextImageKHR"));
        VkPhysicalDeviceProperties2 physicalDeviceProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        vkGetPhysicalDeviceProperties2(device->physical_device.physical_device, &physicalDeviceProperties);
        supportedPhysicalDeviceInfo.physicalDeviceProperties = physicalDeviceProperties.properties;
        VkPhysicalDeviceFeatures2 physicalDeviceFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2};
        physicalDeviceFeatures.pNext = renderDocCapturing ? supportedPhysicalDeviceInfo.pNextHighestRenderDocCompatibleFeature : supportedPhysicalDeviceInfo.pNextHighestFeature;
        vkGetPhysicalDeviceFeatures2(device->physical_device.physical_device, &physicalDeviceFeatures);
        supportedPhysicalDeviceInfo.physicalDeviceFeatures = physicalDeviceFeatures.features;
        vkGetPhysicalDeviceMemoryProperties(device->physical_device.physical_device, &supportedPhysicalDeviceInfo.physicalDeviceMemoryProperties);
    }

    [[nodiscard]] VkCommandBuffer beginSingleTimeCommands() const {
        VkCommandBufferAllocateInfo allocInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO};
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = *commandPool;
        allocInfo.commandBufferCount = 1;
        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device->device, &allocInfo, &commandBuffer);
        VkCommandBufferBeginInfo beginInfo{VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO};
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        return commandBuffer;
    }

    void endSingleTimeCommands(VkCommandBuffer commandBuffer) const {
        vkEndCommandBuffer(commandBuffer);
        VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;
        vkQueueSubmit(*graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(*graphicsQueue);
        vkFreeCommandBuffers(device->device, *commandPool, 1, &commandBuffer);
    }

    VkBool32 enableFeature(VkBool32 *feature) {
        *feature = *testFeature(feature);
        return *feature;
    }

    std::vector<VkBool32> enableFeature(const std::vector<VkBool32 *> &features) {
        std::vector<VkBool32> results{};
        results.reserve(static_cast<unsigned int>(features.size() + 1));
        results[0] = VK_TRUE;
        for (VkBool32 *feature : features) { results.push_back(enableFeature(feature)); }
        for (VkBool32 result : results) {
            if (!result) {
                results[0] = VK_FALSE;
                break;
            }
        }
        return results;
    }

    VkBool32 *testFeature(const VkBool32 *feature) {
        return feature - (VkBool32 *)&enabledPhysicalDeviceInfo + (VkBool32 *)&supportedPhysicalDeviceInfo;
    }

    std::vector<VkBool32> testFeature(const std::vector<VkBool32 *> &features) {
        std::vector<VkBool32> results{};
        results.reserve(static_cast<unsigned int>(features.size() + 1));
        results[0] = VK_TRUE;
        for (VkBool32 *feature : features) { results.push_back(*testFeature(feature)); }
        for (VkBool32 result : results) {
            if (!result) {
                results[0] = VK_FALSE;
                break;
            }
        }
        return results;
    }
};
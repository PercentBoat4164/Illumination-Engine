#pragma once

#include <vector>

#include <VkBootstrap.h>
#include <vk_mem_alloc.h>

#include "CommandBufferManager.hpp"

class VulkanGraphicsEngineLink {
public:
    struct PhysicalDeviceInfo {
        VkPhysicalDeviceRayTracingPipelinePropertiesKHR physicalDeviceRayTracingPipelineProperties{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR};
        VkPhysicalDeviceAccelerationStructureFeaturesKHR physicalDeviceAccelerationStructureFeatures{VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR};
    };

    Settings *settings = nullptr;
    vkb::Device *device{};
    PhysicalDeviceInfo *physicalDeviceInfo{};
    vkb::Swapchain *swapchain{};
    VkCommandPool *commandPool{};
    VmaAllocator *allocator{};
    std::vector<VkImageView> *swapchainImageViews{};
    PFN_vkGetBufferDeviceAddress vkGetBufferDeviceAddressKHR = reinterpret_cast<PFN_vkGetBufferDeviceAddressKHR>(vkGetDeviceProcAddr(device->device, "vkGetBufferDeviceAddressKHR"));
    PFN_vkCmdBuildAccelerationStructuresKHR vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device->device, "vkCmdBuildAccelerationStructuresKHR"));
    PFN_vkBuildAccelerationStructuresKHR vkBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device->device, "vkBuildAccelerationStructuresKHR"));
    PFN_vkCreateAccelerationStructureKHR vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(device->device, "vkCreateAccelerationStructureKHR"));
    PFN_vkDestroyAccelerationStructureKHR vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(device->device, "vkDestroyAccelerationStructureKHR"));
    PFN_vkGetAccelerationStructureBuildSizesKHR vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(device->device, "vkGetAccelerationStructureBuildSizesKHR"));
    PFN_vkGetAccelerationStructureDeviceAddressKHR vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(device->device, "vkGetAccelerationStructureDeviceAddressKHR"));
    PFN_vkCmdTraceRaysKHR vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(device->device, "vkCmdTraceRaysKHR"));
    PFN_vkGetRayTracingShaderGroupHandlesKHR vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(device->device, "vkGetRayTracingShaderGroupHandlesKHR"));
    PFN_vkCreateRayTracingPipelinesKHR vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(device->device, "vkCreateRayTracingPipelinesKHR"));

    [[nodiscard]] VkCommandBuffer beginSingleTimeCommands() const {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = *commandPool;
        allocInfo.commandBufferCount = 1;
        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(device->device, &allocInfo, &commandBuffer);
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
        vkQueueSubmit(device->get_queue(vkb::QueueType::graphics).value(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(device->get_queue(vkb::QueueType::graphics).value());
        vkFreeCommandBuffers(device->device, *commandPool, 1, &commandBuffer);
    }
};
#pragma once

#include <deque>
#include <functional>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "AccelerationStructureManager.hpp"
#include "Asset.hpp"
#include "Vertex.hpp"

struct DescriptorSetManagerCreateInfo {

};

class DescriptorSetManager {
public:
    VkDescriptorPool descriptorPool{};
    VkDescriptorSet descriptorSet{};
    VkDescriptorSetLayout descriptorSetLayout{};

    void destroy() {
        for (const std::function<void()>& function : deletionQueue) { function(); }
        deletionQueue.clear();
    }
    void setup(VulkanGraphicsEngineLink *renderEngineLink, const std::vector<VkDescriptorPoolSize> &poolSizes, const AccelerationStructureManager& accelerationStructure) {
        linkedRenderEngine = renderEngineLink;
        //Create descriptor layout from bindings
        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        descriptorPoolCreateInfo.poolSizeCount = poolSizes.size();
        descriptorPoolCreateInfo.pPoolSizes = poolSizes.data();
        descriptorPoolCreateInfo.maxSets = 1;
        if (vkCreateDescriptorPool(linkedRenderEngine->device->device, &descriptorPoolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS) { throw std::runtime_error("failed to create descriptor pool!"); }
        deletionQueue.emplace_front([&]{ vkDestroyDescriptorPool(linkedRenderEngine->device->device, descriptorPool, nullptr); descriptorPool = VK_NULL_HANDLE; });
        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        descriptorSetAllocateInfo.descriptorPool = descriptorPool;
        descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
        descriptorSetAllocateInfo.descriptorSetCount = 1;
        if (vkAllocateDescriptorSets(linkedRenderEngine->device->device, &descriptorSetAllocateInfo, &descriptorSet) != VK_SUCCESS) { throw std::runtime_error("failed to create descriptor set!"); }
        for (VkDescriptorPoolSize poolSize : poolSizes) {
            if (poolSize.type == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR) {
                VkWriteDescriptorSetAccelerationStructureKHR descriptorSetAccelerationStructure{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR};
                descriptorSetAccelerationStructure.accelerationStructureCount = 1;
                descriptorSetAccelerationStructure.pAccelerationStructures = &accelerationStructure.accelerationStructure;
            }
        }
    }

private:
    VulkanGraphicsEngineLink *linkedRenderEngine{};
    std::deque<std::function<void()>> deletionQueue{};
    std::vector<VkDescriptorType> descriptorTypes{};
    std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings{};
    std::vector<VkDescriptorPoolSize> descriptorPoolSizes{};
};
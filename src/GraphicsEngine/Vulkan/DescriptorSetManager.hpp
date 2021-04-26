#pragma once

#include <deque>
#include <functional>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "AccelerationStructureManager.hpp"
#include "Asset.hpp"
#include "Vertex.hpp"

struct DescriptorSetManagerCreateInfo {
    std::vector<VkDescriptorPoolSize> poolSizes{};
    std::vector<AccelerationStructureManager> accelerationStructures{};
    std::vector<ImageManager> images{};
    std::vector<BufferManager> buffers{};
};

class DescriptorSetManager {
public:
    VkDescriptorPool descriptorPool{};
    VkDescriptorSet descriptorSet{};
    VkDescriptorSetLayout descriptorSetLayout{};
    DescriptorSetManagerCreateInfo descriptorSetManagerCreateInfo{};

    void destroy() {
        for (const std::function<void()>& function : deletionQueue) { function(); }
        deletionQueue.clear();
    }
    void setup(VulkanGraphicsEngineLink *renderEngineLink, DescriptorSetManagerCreateInfo createInfo) {
        linkedRenderEngine = renderEngineLink;
        descriptorSetManagerCreateInfo = createInfo;
        //Create descriptor layout from bindings
        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        descriptorPoolCreateInfo.poolSizeCount = createInfo.poolSizes.size();
        descriptorPoolCreateInfo.pPoolSizes = createInfo.poolSizes.data();
        descriptorPoolCreateInfo.maxSets = 1;
        if (vkCreateDescriptorPool(linkedRenderEngine->device->device, &descriptorPoolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS) { throw std::runtime_error("failed to create descriptor pool!"); }
        deletionQueue.emplace_front([&]{ vkDestroyDescriptorPool(linkedRenderEngine->device->device, descriptorPool, nullptr); descriptorPool = VK_NULL_HANDLE; });
        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        descriptorSetAllocateInfo.descriptorPool = descriptorPool;
        descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
        descriptorSetAllocateInfo.descriptorSetCount = 1;
        if (vkAllocateDescriptorSets(linkedRenderEngine->device->device, &descriptorSetAllocateInfo, &descriptorSet) != VK_SUCCESS) { throw std::runtime_error("failed to create descriptor set!"); }
        std::vector<VkWriteDescriptorSet> descriptorWrites{};
        descriptorWrites.reserve(descriptorSetManagerCreateInfo.poolSizes.size());
        for (unsigned long i = 0; i < descriptorSetManagerCreateInfo.poolSizes.size(); ++i) {
            VkWriteDescriptorSet writeDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            writeDescriptorSet.dstSet = descriptorSet;
            writeDescriptorSet.descriptorType = descriptorSetManagerCreateInfo.poolSizes[i].type;
            writeDescriptorSet.dstBinding = i;
            if (descriptorSetManagerCreateInfo.poolSizes[i].type == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR) {
                VkWriteDescriptorSetAccelerationStructureKHR writeDescriptorSetAccelerationStructure{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR};
                writeDescriptorSetAccelerationStructure.accelerationStructureCount = 1;
                writeDescriptorSetAccelerationStructure.pAccelerationStructures = &descriptorSetManagerCreateInfo.accelerationStructures[i].accelerationStructure;
                writeDescriptorSet.pNext = &writeDescriptorSetAccelerationStructure;
            } else if (descriptorSetManagerCreateInfo.poolSizes[i].type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
                VkDescriptorImageInfo storageImageDescriptorInfo{};
                storageImageDescriptorInfo.imageView = descriptorSetManagerCreateInfo.images[i].view;
                storageImageDescriptorInfo.sampler = descriptorSetManagerCreateInfo.images[i].sampler;
                storageImageDescriptorInfo.imageLayout = descriptorSetManagerCreateInfo.images[i].imageLayout;
                if (storageImageDescriptorInfo.imageView == VK_NULL_HANDLE) { throw std::runtime_error("given image does not have an associated view!"); }
                writeDescriptorSet.pImageInfo = &storageImageDescriptorInfo;
            } else if (descriptorSetManagerCreateInfo.poolSizes[i].type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                VkDescriptorImageInfo combinedImageSamplerDescriptorInfo{};
                combinedImageSamplerDescriptorInfo.imageView = descriptorSetManagerCreateInfo.images[i].view;
                combinedImageSamplerDescriptorInfo.sampler = descriptorSetManagerCreateInfo.images[i].sampler;
                combinedImageSamplerDescriptorInfo.imageLayout = descriptorSetManagerCreateInfo.images[i].imageLayout;
                if (combinedImageSamplerDescriptorInfo.sampler == VK_NULL_HANDLE) { throw std::runtime_error("given image does not have an associated sampler!"); }
                writeDescriptorSet.pImageInfo = &combinedImageSamplerDescriptorInfo;
            } else if (descriptorSetManagerCreateInfo.poolSizes[i].type == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
                VkDescriptorBufferInfo storageBufferDescriptorInfo{};
                storageBufferDescriptorInfo.buffer = descriptorSetManagerCreateInfo.buffers[i].buffer;
                storageBufferDescriptorInfo.offset = 0;
                storageBufferDescriptorInfo.range = VK_WHOLE_SIZE;
                if (storageBufferDescriptorInfo.buffer == VK_NULL_HANDLE) { throw std::runtime_error("given buffer has not been created!"); }
                writeDescriptorSet.pBufferInfo = &storageBufferDescriptorInfo;
            } else if (descriptorSetManagerCreateInfo.poolSizes[i].type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                VkDescriptorBufferInfo storageBufferDescriptorInfo{};
                storageBufferDescriptorInfo.buffer = descriptorSetManagerCreateInfo.buffers[i].buffer;
                storageBufferDescriptorInfo.offset = 0;
                storageBufferDescriptorInfo.range = VK_WHOLE_SIZE;
                if (storageBufferDescriptorInfo.buffer == VK_NULL_HANDLE) { throw std::runtime_error("given buffer has not been created!"); }
                writeDescriptorSet.pBufferInfo = &storageBufferDescriptorInfo;
            } else { throw std::runtime_error("unsupported descriptor type!"); }
            descriptorWrites.push_back(writeDescriptorSet);
        }
        vkUpdateDescriptorSets(linkedRenderEngine->device->device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
    }

private:
    VulkanGraphicsEngineLink *linkedRenderEngine{};
    std::deque<std::function<void()>> deletionQueue{};
};
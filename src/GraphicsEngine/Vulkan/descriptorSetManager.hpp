#pragma once

#include <deque>
#include <functional>
#include <vector>

#include <vulkan/vulkan.hpp>

#include "accelerationStructureManager.hpp"
#include "asset.hpp"
#include "vertex.hpp"

class DescriptorSetManager {
public:
    struct DescriptorSetManagerCreateInfo {
        std::vector<VkDescriptorPoolSize> poolSizes{};
        std::vector<VkShaderStageFlagBits> shaderStages{};
        std::vector<std::vector<char>> shaderData{};
        std::vector<AccelerationStructureManager *> accelerationStructures{};
        std::vector<ImageManager *> images{};
        std::vector<BufferManager *> buffers{};
    };

    VkDescriptorPool descriptorPool{};
    VkDescriptorSet descriptorSet{};
    VkDescriptorSetLayout descriptorSetLayout{};
    DescriptorSetManagerCreateInfo *createdWith{};

    void destroy() {
        for (const std::function<void()>& function : deletionQueue) { function(); }
        deletionQueue.clear();
    }

    void create(VulkanGraphicsEngineLink *renderEngineLink, DescriptorSetManagerCreateInfo *createInfo) {
        linkedRenderEngine = renderEngineLink;
        createdWith = createInfo;
        //Create descriptor layout from bindings
        std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings{};
        descriptorSetLayoutBindings.reserve(createdWith->poolSizes.size());
        for (unsigned long i = 0; i < createdWith->poolSizes.size(); ++i) {
            VkDescriptorSetLayoutBinding descriptorSetLayoutBinding{};
            descriptorSetLayoutBinding.descriptorType = createdWith->poolSizes[i].type;
            descriptorSetLayoutBinding.binding = i;
            descriptorSetLayoutBinding.stageFlags = createdWith->shaderStages[i];
            descriptorSetLayoutBinding.descriptorCount = 1;
            descriptorSetLayoutBindings.push_back(descriptorSetLayoutBinding);
        }
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
        if (vkCreateDescriptorSetLayout(linkedRenderEngine->device->device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) { throw std::runtime_error("failed to create descriptor layout!"); }
        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(createdWith->poolSizes.size());
        descriptorPoolCreateInfo.pPoolSizes = createdWith->poolSizes.data();
        descriptorPoolCreateInfo.maxSets = 1;
        if (vkCreateDescriptorPool(linkedRenderEngine->device->device, &descriptorPoolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS) { throw std::runtime_error("failed to create descriptor pool!"); }
        deletionQueue.emplace_front([&]{ vkDestroyDescriptorPool(linkedRenderEngine->device->device, descriptorPool, nullptr); descriptorPool = VK_NULL_HANDLE; });
        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        descriptorSetAllocateInfo.descriptorPool = descriptorPool;
        descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
        descriptorSetAllocateInfo.descriptorSetCount = 1;
        if (vkAllocateDescriptorSets(linkedRenderEngine->device->device, &descriptorSetAllocateInfo, &descriptorSet) != VK_SUCCESS) { throw std::runtime_error("failed to create descriptor set!"); }
        std::vector<VkWriteDescriptorSet> descriptorWrites{};
        descriptorWrites.reserve(createdWith->poolSizes.size());
        for (unsigned long i = 0; i < createdWith->poolSizes.size(); ++i) {
            VkWriteDescriptorSet writeDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
            writeDescriptorSet.dstSet = descriptorSet;
            writeDescriptorSet.descriptorType = createdWith->poolSizes[i].type;
            writeDescriptorSet.dstBinding = i;
            if (writeDescriptorSet.descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR) {
                VkWriteDescriptorSetAccelerationStructureKHR writeDescriptorSetAccelerationStructure{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR};
                writeDescriptorSetAccelerationStructure.accelerationStructureCount = 1;
                writeDescriptorSetAccelerationStructure.pAccelerationStructures = &createdWith->accelerationStructures[i]->accelerationStructure;
                writeDescriptorSet.pNext = &writeDescriptorSetAccelerationStructure;
            } else if (writeDescriptorSet.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
                VkDescriptorImageInfo storageImageDescriptorInfo{};
                storageImageDescriptorInfo.imageView = createdWith->images[i]->view;
                storageImageDescriptorInfo.sampler = createdWith->images[i]->sampler;
                storageImageDescriptorInfo.imageLayout = createdWith->images[i]->imageLayout;
                if (storageImageDescriptorInfo.imageView == VK_NULL_HANDLE) { throw std::runtime_error("given image does not have an associated view!"); }
                writeDescriptorSet.pImageInfo = &storageImageDescriptorInfo;
            } else if (writeDescriptorSet.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                VkDescriptorImageInfo combinedImageSamplerDescriptorInfo{};
                combinedImageSamplerDescriptorInfo.imageView = createdWith->images[i]->view;
                combinedImageSamplerDescriptorInfo.sampler = createdWith->images[i]->sampler;
                combinedImageSamplerDescriptorInfo.imageLayout = createdWith->images[i]->imageLayout;
                if (combinedImageSamplerDescriptorInfo.sampler == VK_NULL_HANDLE) { throw std::runtime_error("given image does not have an associated sampler!"); }
                writeDescriptorSet.pImageInfo = &combinedImageSamplerDescriptorInfo;
            } else if (writeDescriptorSet.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
                VkDescriptorBufferInfo storageBufferDescriptorInfo{};
                storageBufferDescriptorInfo.buffer = createdWith->buffers[i]->buffer;
                storageBufferDescriptorInfo.offset = 0;
                storageBufferDescriptorInfo.range = VK_WHOLE_SIZE;
                if (storageBufferDescriptorInfo.buffer == VK_NULL_HANDLE) { throw std::runtime_error("given buffer has not been created!"); }
                writeDescriptorSet.pBufferInfo = &storageBufferDescriptorInfo;
            } else if (writeDescriptorSet.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                VkDescriptorBufferInfo storageBufferDescriptorInfo{};
                storageBufferDescriptorInfo.buffer = createdWith->buffers[i]->buffer;
                storageBufferDescriptorInfo.offset = 0;
                storageBufferDescriptorInfo.range = VK_WHOLE_SIZE;
                if (storageBufferDescriptorInfo.buffer == VK_NULL_HANDLE) { throw std::runtime_error("given buffer has not been created!"); }
                writeDescriptorSet.pBufferInfo = &storageBufferDescriptorInfo;
            } else { throw std::runtime_error("unsupported descriptor type!"); }
            descriptorWrites.push_back(writeDescriptorSet);
        }
        vkUpdateDescriptorSets(linkedRenderEngine->device->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

private:
    VulkanGraphicsEngineLink *linkedRenderEngine{};
    std::deque<std::function<void()>> deletionQueue{};
};
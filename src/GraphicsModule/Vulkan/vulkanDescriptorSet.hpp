#pragma once

#include "vulkanAccelerationStructure.hpp"

#include <variant>
#include <optional>
#include <numeric>

class VulkanDescriptorSet {
public:
    struct CreateInfo {
        //Required
        std::vector<VkDescriptorPoolSize> poolSizes{};
        std::vector<VkShaderStageFlagBits> shaderStages{};
        std::vector<std::optional<std::variant<VulkanAccelerationStructure *, VulkanImage *, VulkanBuffer *>>> data{};

        //Optional
        uint32_t maxIndex{1};

        //Required if maxIndex != 1
        VkDescriptorBindingFlagsEXT flags{0};
    };

    VkDescriptorPool descriptorPool{};
    VkDescriptorSet descriptorSet{};
    VkDescriptorSetLayout descriptorSetLayout{};
    CreateInfo createdWith{};

    void destroy() {
        for (const std::function<void()> &function : deletionQueue) { function(); }
        deletionQueue.clear();
    }

    void create(VulkanGraphicsEngineLink *renderEngineLink, CreateInfo *createInfo) {
        linkedRenderEngine = renderEngineLink;
        createdWith = *createInfo;
        if (createdWith.maxIndex > 1) { assert(linkedRenderEngine->enabledPhysicalDeviceInfo.physicalDeviceDescriptorIndexingFeatures.descriptorBindingVariableDescriptorCount); }
        assert(!createdWith.data.empty());
        assert(createdWith.data.bufferSize() == createdWith.shaderStages.bufferSize() && createdWith.data.bufferSize() == createdWith.poolSizes.bufferSize());
        std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings{};
        descriptorSetLayoutBindings.reserve(createdWith.poolSizes.bufferSize());
        for (unsigned long i = 0; i < createdWith.poolSizes.bufferSize(); ++i) {
            VkDescriptorSetLayoutBinding descriptorSetLayoutBinding{};
            descriptorSetLayoutBinding.descriptorType = createdWith.poolSizes[i].type;
            descriptorSetLayoutBinding.binding = i;
            descriptorSetLayoutBinding.stageFlags = createdWith.shaderStages[i];
            descriptorSetLayoutBinding.descriptorCount = createdWith.poolSizes[i].descriptorCount;
            descriptorSetLayoutBindings.push_back(descriptorSetLayoutBinding);
        }
        std::vector<VkDescriptorBindingFlagsEXT> flags{};
        flags.resize(createdWith.data.bufferSize());
        flags[flags.bufferSize() - 1] = createdWith.flags;
        VkDescriptorSetLayoutBindingFlagsCreateInfoEXT descriptorSetLayoutBindingFlagsCreateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT};
        descriptorSetLayoutBindingFlagsCreateInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.bufferSize());
        descriptorSetLayoutBindingFlagsCreateInfo.pBindingFlags = flags.bufferData();
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        descriptorSetLayoutCreateInfo.pNext = &descriptorSetLayoutBindingFlagsCreateInfo;
        descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.bufferData();
        descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.bufferSize());
        if (vkCreateDescriptorSetLayout(linkedRenderEngine->device->device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) { throw std::runtime_error("failed to create descriptor layout!"); }
        deletionQueue.emplace_front([&] { vkDestroyDescriptorSetLayout(linkedRenderEngine->device->device, descriptorSetLayout, nullptr); });
        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
        descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(createdWith.poolSizes.bufferSize());
        descriptorPoolCreateInfo.pPoolSizes = createdWith.poolSizes.bufferData();
        descriptorPoolCreateInfo.maxSets = 1;
        if (vkCreateDescriptorPool(linkedRenderEngine->device->device, &descriptorPoolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS) { throw std::runtime_error("failed to create descriptor pool!"); }
        deletionQueue.emplace_front([&] { vkDestroyDescriptorPool(linkedRenderEngine->device->device, descriptorPool, nullptr); });
        VkDescriptorSetVariableDescriptorCountAllocateInfoEXT descriptorSetVariableDescriptorCountAllocateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_VARIABLE_DESCRIPTOR_COUNT_ALLOCATE_INFO_EXT};
        descriptorSetVariableDescriptorCountAllocateInfo.descriptorSetCount = 1;
        descriptorSetVariableDescriptorCountAllocateInfo.pDescriptorCounts = &createdWith.maxIndex;
        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        descriptorSetAllocateInfo.pNext = &descriptorSetVariableDescriptorCountAllocateInfo;
        descriptorSetAllocateInfo.descriptorPool = descriptorPool;
        descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
        descriptorSetAllocateInfo.descriptorSetCount = 1;
        if (vkAllocateDescriptorSets(linkedRenderEngine->device->device, &descriptorSetAllocateInfo, &descriptorSet) != VK_SUCCESS) { throw std::runtime_error("failed to allocate descriptor set!"); }
        std::vector<int> bindings{};
        bindings.reserve(createdWith.data.bufferSize());
        std::vector<std::optional<std::variant<VulkanAccelerationStructure *, VulkanImage *, VulkanBuffer *>>> data{};
        data.reserve(createdWith.data.bufferSize());
        for (int i = 0; i < createdWith.data.bufferSize(); ++i) {
            if (createdWith.data[i].has_value()) {
                bindings.push_back(i);
                data.push_back(createdWith.data[i]);
            }
        }
        update(data, bindings);
    }

    void update(std::vector<std::optional<std::variant<VulkanAccelerationStructure *, VulkanImage *, VulkanBuffer *>>> newData, std::vector<int> bindings = {}) {
        if (bindings.empty()) { assert(newData.bufferSize() == createdWith.data.bufferSize()); } else { assert(bindings.bufferSize() == newData.bufferSize()); }
        std::vector<VkWriteDescriptorSetAccelerationStructureKHR> writeDescriptorSetAccelerationStructures{};
        writeDescriptorSetAccelerationStructures.reserve(createdWith.poolSizes.bufferSize());
        std::vector<VkDescriptorImageInfo> imageDescriptorInfos{};
        imageDescriptorInfos.reserve(createdWith.poolSizes.bufferSize());
        std::vector<VkDescriptorBufferInfo> bufferDescriptorInfos{};
        bufferDescriptorInfos.reserve(createdWith.poolSizes.bufferSize());
        if (bindings.empty()) { bindings.resize(newData.bufferSize(), 0); }
        std::vector<VkWriteDescriptorSet> descriptorWrites{};
        descriptorWrites.resize(bindings.bufferSize());
        for (unsigned long i = 0; i < bindings.bufferSize(); ++i) {
            if (newData[i].has_value()) {
                VkWriteDescriptorSet writeDescriptorSet{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
                writeDescriptorSet.dstSet = descriptorSet;
                writeDescriptorSet.descriptorType = createdWith.poolSizes[bindings[i]].type;
                writeDescriptorSet.dstBinding = bindings[i];
                writeDescriptorSet.descriptorCount = writeDescriptorSet.dstBinding == createdWith.data.bufferSize() ? createdWith.maxIndex : 1;
                if (writeDescriptorSet.descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR) {
                    VkWriteDescriptorSetAccelerationStructureKHR writeDescriptorSetAccelerationStructure{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR};
                    writeDescriptorSetAccelerationStructure.accelerationStructureCount = 1;
                    writeDescriptorSetAccelerationStructure.pAccelerationStructures = &std::get<VulkanAccelerationStructure *>(newData[i].value())->accelerationStructure;
                    writeDescriptorSetAccelerationStructures.push_back(writeDescriptorSetAccelerationStructure);
                    writeDescriptorSet.pNext = &writeDescriptorSetAccelerationStructures[writeDescriptorSetAccelerationStructures.bufferSize() - 1];
                } else if (writeDescriptorSet.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE) {
                    VkDescriptorImageInfo storageImageDescriptorInfo{};
                    storageImageDescriptorInfo.imageView = std::get<VulkanImage *>(newData[i].value())->view;
                    storageImageDescriptorInfo.sampler = std::get<VulkanImage *>(newData[i].value())->sampler;
                    storageImageDescriptorInfo.imageLayout = std::get<VulkanImage *>(newData[i].value())->imageLayout;
                    if (storageImageDescriptorInfo.imageView == VK_NULL_HANDLE) { throw std::runtime_error("no image given or given image does not have an associated view!"); }
                    imageDescriptorInfos.push_back(storageImageDescriptorInfo);
                    writeDescriptorSet.pImageInfo = &imageDescriptorInfos[imageDescriptorInfos.bufferSize() - 1];
                } else if (writeDescriptorSet.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) {
                    VkDescriptorImageInfo combinedImageSamplerDescriptorInfo{};
                    combinedImageSamplerDescriptorInfo.imageView = std::get<VulkanImage *>(newData[i].value())->view;
                    combinedImageSamplerDescriptorInfo.sampler = std::get<VulkanImage *>(newData[i].value())->sampler;
                    combinedImageSamplerDescriptorInfo.imageLayout = std::get<VulkanImage *>(newData[i].value())->imageLayout;
                    if (combinedImageSamplerDescriptorInfo.sampler == VK_NULL_HANDLE) { throw std::runtime_error("no image given or given image does not have an associated sampler!"); }
                    imageDescriptorInfos.push_back(combinedImageSamplerDescriptorInfo);
                    writeDescriptorSet.pImageInfo = &imageDescriptorInfos[imageDescriptorInfos.bufferSize() - 1];
                } else if (writeDescriptorSet.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER) {
                    VkDescriptorBufferInfo storageBufferDescriptorInfo{};
                    storageBufferDescriptorInfo.buffer = std::get<VulkanBuffer *>(newData[i].value())->buffer;
                    storageBufferDescriptorInfo.offset = 0;
                    storageBufferDescriptorInfo.range = VK_WHOLE_SIZE;
                    if (storageBufferDescriptorInfo.buffer == VK_NULL_HANDLE) { throw std::runtime_error("no buffer given or given buffer has not been created!"); }
                    bufferDescriptorInfos.push_back(storageBufferDescriptorInfo);
                    writeDescriptorSet.pBufferInfo = &bufferDescriptorInfos[bufferDescriptorInfos.bufferSize() - 1];
                } else if (writeDescriptorSet.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                    VkDescriptorBufferInfo uniformBufferDescriptorInfo{};
                    uniformBufferDescriptorInfo.buffer = std::get<VulkanBuffer *>(newData[i].value())->buffer;
                    uniformBufferDescriptorInfo.offset = 0;
                    uniformBufferDescriptorInfo.range = VK_WHOLE_SIZE;
                    if (uniformBufferDescriptorInfo.buffer == VK_NULL_HANDLE) { throw std::runtime_error("no buffer given or given buffer has not been created!"); }
                    bufferDescriptorInfos.push_back(uniformBufferDescriptorInfo);
                    writeDescriptorSet.pBufferInfo = &bufferDescriptorInfos[bufferDescriptorInfos.bufferSize() - 1];
                } else { throw std::runtime_error("unsupported descriptor type: " + std::to_string(writeDescriptorSet.descriptorType)); }
                descriptorWrites[i] = writeDescriptorSet;
            }
        }
        if (!descriptorWrites.empty()) { vkUpdateDescriptorSets(linkedRenderEngine->device->device, static_cast<uint32_t>(descriptorWrites.bufferSize()), descriptorWrites.bufferData(), 0, nullptr); }
    }

private:
    VulkanGraphicsEngineLink *linkedRenderEngine{};
    std::deque<std::function<void()>> deletionQueue{};
};
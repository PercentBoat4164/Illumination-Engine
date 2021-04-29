#pragma once

#include <vulkan/vulkan.hpp>

#include "DescriptorSetManager.hpp"
#include "VulkanGraphicsEngineLink.hpp"

class RayTracingPipelineManager {
    VkPipelineLayout pipelineLayout{};
    DescriptorSetManager descriptorSetManager{};
    VulkanGraphicsEngineLink *linkedRenderEngine{};
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};

    void destroy() {
        for (const std::function<void()>& function : deletionQueue) { function(); }
        deletionQueue.clear();
    }

    void create(const DescriptorSetManagerCreateInfo& descriptorSetManagerCreateInfo) {
        descriptorSetManager.create(linkedRenderEngine, descriptorSetManagerCreateInfo);
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetManager.descriptorSetLayout;
        if (vkCreatePipelineLayout(linkedRenderEngine->device->device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) { throw std::runtime_error("failed to create pipeline layout!"); }
        deletionQueue.emplace_front([&] { vkDestroyPipelineLayout(linkedRenderEngine->device->device, pipelineLayout, nullptr); });
//        shaderStages.reserve(descriptorSetManager)
//        for ()
    }

private:
    std::deque<std::function<void()>> deletionQueue{};
};
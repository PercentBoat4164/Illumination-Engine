#pragma once

#include "descriptorSetManager.hpp"

class RayTracingPipelineManager {
    VkPipelineLayout pipelineLayout{};
    DescriptorSetManager descriptorSetManager{};
    VulkanGraphicsEngineLink *linkedRenderEngine{};
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};
    VkPipeline pipeline{};

public:
    void create(VulkanGraphicsEngineLink *renderEngineLink, DescriptorSetManager::DescriptorSetManagerCreateInfo *descriptorSetManagerCreateInfo) {
        linkedRenderEngine = renderEngineLink;
        descriptorSetManager.create(linkedRenderEngine, descriptorSetManagerCreateInfo);
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetManager.descriptorSetLayout;
        if (vkCreatePipelineLayout(linkedRenderEngine->device->device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) { throw std::runtime_error("failed to create pipeline layout!"); }
        deletionQueue.emplace_front([&] { vkDestroyPipelineLayout(linkedRenderEngine->device->device, pipelineLayout, nullptr); });
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};
        shaderStages.reserve(descriptorSetManager.createdWith->shaderStages.size());
        shaderGroups.reserve(descriptorSetManager.createdWith->shaderStages.size());
        for (unsigned int i = 0; i > descriptorSetManager.createdWith->shaderStages.size(); ++i) {
            VkShaderModule shaderModule;
            VkShaderModuleCreateInfo shaderModuleCreateInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
            shaderModuleCreateInfo.codeSize = descriptorSetManager.createdWith->shaderData[i].size();
            shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t *>(descriptorSetManager.createdWith->shaderData[i].data());
            if (vkCreateShaderModule(linkedRenderEngine->device->device, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
                for (VkPipelineShaderStageCreateInfo shader : shaderStages) { vkDestroyShaderModule(linkedRenderEngine->device->device, shader.module, nullptr); }
                throw std::runtime_error("failed to create shader module!");
            }
            VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
            pipelineShaderStageCreateInfo.stage = descriptorSetManager.createdWith->shaderStages[i];
            pipelineShaderStageCreateInfo.module = shaderModule;
            pipelineShaderStageCreateInfo.pName = "main";
            shaderStages.push_back(pipelineShaderStageCreateInfo);
            VkRayTracingShaderGroupCreateInfoKHR rayTracingShaderGroupCreateInfo{VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR};
            rayTracingShaderGroupCreateInfo.generalShader = VK_SHADER_UNUSED_KHR;
            rayTracingShaderGroupCreateInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
            rayTracingShaderGroupCreateInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
            rayTracingShaderGroupCreateInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
            if (pipelineShaderStageCreateInfo.stage == VK_SHADER_STAGE_RAYGEN_BIT_KHR) {
                rayTracingShaderGroupCreateInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
                rayTracingShaderGroupCreateInfo.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
            } else if (pipelineShaderStageCreateInfo.stage == VK_SHADER_STAGE_MISS_BIT_KHR) {
                rayTracingShaderGroupCreateInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
                rayTracingShaderGroupCreateInfo.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
            } else if (pipelineShaderStageCreateInfo.stage == VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR) {
                rayTracingShaderGroupCreateInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
                rayTracingShaderGroupCreateInfo.closestHitShader = static_cast<uint32_t>(shaderStages.size()) - 1;
            } else if (pipelineShaderStageCreateInfo.stage == VK_SHADER_STAGE_CALLABLE_BIT_KHR) {
                rayTracingShaderGroupCreateInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
                rayTracingShaderGroupCreateInfo.generalShader = static_cast<uint32_t>(shaderStages.size()) - 1;
            }
            shaderGroups.push_back(rayTracingShaderGroupCreateInfo);
        }
        VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCreateInfo{VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR};
        rayTracingPipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
        rayTracingPipelineCreateInfo.pStages = shaderStages.data();
        rayTracingPipelineCreateInfo.groupCount = static_cast<uint32_t>(shaderGroups.size());
        rayTracingPipelineCreateInfo.pGroups = shaderGroups.data();
        rayTracingPipelineCreateInfo.maxPipelineRayRecursionDepth = 2;
        rayTracingPipelineCreateInfo.layout = pipelineLayout;
        linkedRenderEngine->vkCreateRayTracingPipelinesKHR(linkedRenderEngine->device->device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayTracingPipelineCreateInfo, nullptr, &pipeline);
        for (VkPipelineShaderStageCreateInfo shader : shaderStages) { vkDestroyShaderModule(linkedRenderEngine->device->device, shader.module, nullptr); }
    }

    void destroy() {
        for (const std::function<void()>& function : deletionQueue) { function(); }
        deletionQueue.clear();
    }

private:
    std::deque<std::function<void()>> deletionQueue{};
};
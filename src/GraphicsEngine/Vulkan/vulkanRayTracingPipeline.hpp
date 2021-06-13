#pragma once

#include "vulkanShader.hpp"
#include "vulkanDescriptorSet.hpp"

class RayTracingPipeline {
public:
    struct CreateInfo {
        std::vector<Shader> shaders{};
        DescriptorSetManager *descriptorSetManager{};
    };

    VkPipelineLayout pipelineLayout{};
    VulkanGraphicsEngineLink *linkedRenderEngine{};
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups{};
    VkPipeline pipeline{};
    CreateInfo *createdWith{};

    void create(VulkanGraphicsEngineLink *renderEngineLink, CreateInfo *createInfo) {
        linkedRenderEngine = renderEngineLink;
        createdWith = createInfo;
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &createdWith->descriptorSetManager->descriptorSetLayout;
        if (vkCreatePipelineLayout(linkedRenderEngine->device->device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) { throw std::runtime_error("failed to create pipeline layout!"); }
        deletionQueue.emplace_front([&] { vkDestroyPipelineLayout(linkedRenderEngine->device->device, pipelineLayout, nullptr); });
        std::vector<VkPipelineShaderStageCreateInfo> shaderStages{};
        shaderGroups.reserve(createdWith->shaders.size());
        VkRayTracingShaderGroupCreateInfoKHR rayTracingShaderGroupCreateInfo{VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR};
        for (auto & shader : createdWith->shaders) {
            VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
            pipelineShaderStageCreateInfo.stage = shader.createdWith->stage;
            pipelineShaderStageCreateInfo.module = shader.module;
            pipelineShaderStageCreateInfo.pName = "main";
            shaderStages.push_back(pipelineShaderStageCreateInfo);
            rayTracingShaderGroupCreateInfo.closestHitShader = VK_SHADER_UNUSED_KHR;
            rayTracingShaderGroupCreateInfo.intersectionShader = VK_SHADER_UNUSED_KHR;
            rayTracingShaderGroupCreateInfo.anyHitShader = VK_SHADER_UNUSED_KHR;
            rayTracingShaderGroupCreateInfo.generalShader = VK_SHADER_UNUSED_KHR;
            if (pipelineShaderStageCreateInfo.stage & (VK_SHADER_STAGE_CALLABLE_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_RAYGEN_BIT_KHR)) {
                rayTracingShaderGroupCreateInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
                rayTracingShaderGroupCreateInfo.generalShader = shaderStages.size() - 1;
            } else if (pipelineShaderStageCreateInfo.stage & VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR) {
                rayTracingShaderGroupCreateInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
                rayTracingShaderGroupCreateInfo.closestHitShader = shaderStages.size() - 1;
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
        deletionQueue.emplace_front([&]{ vkDestroyPipeline(linkedRenderEngine->device->device, pipeline, nullptr); });
        for (VkPipelineShaderStageCreateInfo shader : shaderStages) { vkDestroyShaderModule(linkedRenderEngine->device->device, shader.module, nullptr); }
    }

    void destroy() {
        for (const std::function<void()>& function : deletionQueue) { function(); }
        deletionQueue.clear();
    }

private:
    std::deque<std::function<void()>> deletionQueue{};
};
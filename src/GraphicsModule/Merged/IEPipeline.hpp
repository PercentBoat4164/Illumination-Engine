#pragma once

#include "IERenderPass.hpp"
#include "IEShader.hpp"
#include "IEVertex.hpp"
#include "IERenderEngineLink.hpp"
#include "IEDescriptorSet.hpp"

/**@todo Make this compatible with OpenGL stuff in a better way.*/
#ifdef ILLUMINATION_ENGINE_VULKAN
#include <vulkan/vulkan.hpp>
#endif

#include <vector>

class IEPipeline {
public:
    struct CreateInfo {
        //Required
        std::vector<IEShader> shaders{};
        IEDescriptorSet *descriptorSet{};
        IERenderPass *renderPass{};
    };

    struct Created {
        bool pipeline{};
    };

    IERenderEngineLink* linkedRenderEngine{};
    #ifdef ILLUMINATION_ENGINE_VULKAN
    VkPipelineLayout pipelineLayout{};
    VkPipeline pipeline{};
    #endif
    CreateInfo createdWith{};
    Created created{};

    void create(IERenderEngineLink *engineLink, CreateInfo *createInfo) {
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (linkedRenderEngine->api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
            linkedRenderEngine = engineLink;
            createdWith = *createInfo;
            //Create pipelineLayout
            VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
            pipelineLayoutCreateInfo.setLayoutCount = 1;
            pipelineLayoutCreateInfo.pSetLayouts = &createdWith.descriptorSet->descriptorSetLayout;
            if (vkCreatePipelineLayout(linkedRenderEngine->device.device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) { throw std::runtime_error("failed to create pipeline layout!"); }
            //prepare shaders
            std::vector<VkPipelineShaderStageCreateInfo> shaders{};
            for (unsigned int i = 0; i < createdWith.shaders.size(); i++) {
                VkShaderModule shaderModule;
                VkShaderModuleCreateInfo shaderModuleCreateInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
                shaderModuleCreateInfo.codeSize = createdWith.shaders[i].data.size();
                shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t *>(createdWith.shaders[i].data.data());
                if (vkCreateShaderModule(linkedRenderEngine->device.device, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) { throw std::runtime_error("failed to create shader module!"); }
                VkPipelineShaderStageCreateInfo shaderStageInfo{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
                shaderStageInfo.module = shaderModule;
                shaderStageInfo.pName = "main";
                shaderStageInfo.stage = i % 2 ? VK_SHADER_STAGE_FRAGMENT_BIT : VK_SHADER_STAGE_VERTEX_BIT;
                shaders.push_back(shaderStageInfo);
            }
            //create graphics pipeline
            VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
            VkVertexInputBindingDescription bindingDescription = IEVertex::getBindingDescription();
            std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = IEVertex::getAttributeDescriptions();
            vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
            vertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
            vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;
            vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
            VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
            inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
            inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
            VkViewport viewport{};
            viewport.x = 0.f;
            viewport.y = 0.f;
            viewport.width = (float) linkedRenderEngine->swapchain.extent.width;
            viewport.height = (float) linkedRenderEngine->swapchain.extent.height;
            viewport.minDepth = 0.f;
            viewport.maxDepth = 1.f;
            VkRect2D scissor{};
            scissor.offset = {0, 0};
            scissor.extent = linkedRenderEngine->swapchain.extent;
            VkPipelineViewportStateCreateInfo viewportStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
            viewportStateCreateInfo.viewportCount = 1;
            viewportStateCreateInfo.pViewports = &viewport;
            viewportStateCreateInfo.scissorCount = 1;
            viewportStateCreateInfo.pScissors = &scissor;
            VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
            rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
            rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
            rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; //Controls fill mode (e.g. wireframe mode)
            rasterizationStateCreateInfo.lineWidth = 1.0f;
            rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
            rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
            rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
            VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
            multisampleStateCreateInfo.sampleShadingEnable = (linkedRenderEngine->physicalDevice.enabledAPIComponents.msaaSmoothing & (linkedRenderEngine->settings.msaaSamples != VK_SAMPLE_COUNT_1_BIT)) ? VK_TRUE : VK_FALSE;
            multisampleStateCreateInfo.minSampleShading = 1.0f;
            multisampleStateCreateInfo.rasterizationSamples = static_cast<VkSampleCountFlagBits>(linkedRenderEngine->settings.msaaSamples);
            VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
            pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
            pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
            pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
            pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
            pipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
            pipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
            VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
            colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachmentState.blendEnable = linkedRenderEngine->settings.rayTracing ? VK_FALSE : VK_TRUE;
            colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
            colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
            VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
            colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
            colorBlendStateCreateInfo.attachmentCount = 1;
            colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
            VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
            VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
            pipelineDynamicStateCreateInfo.dynamicStateCount = 2;
            pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStates;
            VkGraphicsPipelineCreateInfo pipelineCreateInfo{VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};
            pipelineCreateInfo.stageCount = 2;
            pipelineCreateInfo.pStages = shaders.data();
            pipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
            pipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
            pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
            pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
            pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
            pipelineCreateInfo.pDepthStencilState = &pipelineDepthStencilStateCreateInfo;
            pipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
            pipelineCreateInfo.pDynamicState = &pipelineDynamicStateCreateInfo;
            pipelineCreateInfo.layout = pipelineLayout;
            pipelineCreateInfo.renderPass = createdWith.renderPass->renderPass;
            pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
            if (vkCreateGraphicsPipelines(linkedRenderEngine->device.device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS) { throw std::runtime_error("failed to create graphics pipeline!"); }
            for (VkPipelineShaderStageCreateInfo shader: shaders) { vkDestroyShaderModule(linkedRenderEngine->device.device, shader.module, nullptr); }
        }
        #endif
        created.pipeline = true;
    }

    void destroy() {
        if (created.pipeline) {
            #ifdef ILLUMINATION_ENGINE_VULKAN
            if (linkedRenderEngine->api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
                vkDestroyPipelineLayout(linkedRenderEngine->device.device, pipelineLayout, nullptr);
                vkDestroyPipeline(linkedRenderEngine->device.device, pipeline, nullptr);
            }
            #endif
        }
    }

    ~IEPipeline() {
        destroy();
    }
};
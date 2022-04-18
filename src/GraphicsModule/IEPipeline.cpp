/* Include this file's header. */
#include "IEPipeline.hpp"

/* Include dependencies within this module. */
#include "IEDescriptorSet.hpp"
#include "IERenderEngine.hpp"
#include "IEVertex.hpp"

/* Include dependencies from Core. */
#include "Core/LogModule/IELogger.hpp"


void IEPipeline::destroy() {
    for (const std::function<void()> &function : deletionQueue) {
        function();
    }
    deletionQueue.clear();
}

void IEPipeline::create(IERenderEngine *engineLink, IEPipeline::CreateInfo *createInfo) {
    linkedRenderEngine = engineLink;
    createdWith = *createInfo;
    // Create pipelineLayout
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    pipelineLayoutCreateInfo.setLayoutCount = 1;
    pipelineLayoutCreateInfo.pSetLayouts = &createdWith.descriptorSet->descriptorSetLayout;
    if (vkCreatePipelineLayout(linkedRenderEngine->device.device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Failed to create pipeline layout!");
    }
    #ifndef NDEBUG
    created.pipelineLayout = true;
    #endif
    deletionQueue.emplace_back([&] {
        #ifndef NDEBUG
        if (created.pipelineLayout) {
            #endif
            vkDestroyPipelineLayout(linkedRenderEngine->device.device, pipelineLayout, nullptr);
            #ifndef NDEBUG
            created.pipelineLayout = false;
        }
        #endif
    });
    // prepare shaders
    std::vector<VkPipelineShaderStageCreateInfo> shaders{};
    for (uint32_t i = 0; i < createdWith.shaders->size(); i++) {
        VkPipelineShaderStageCreateInfo shaderStageInfo{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        shaderStageInfo.module = (*createdWith.shaders)[i].module;
        shaderStageInfo.pName = "main";
        shaderStageInfo.stage = (i % 2) != 0U ? VK_SHADER_STAGE_FRAGMENT_BIT : VK_SHADER_STAGE_VERTEX_BIT;
        shaders.push_back(shaderStageInfo);
    }
    // create graphics pipeline
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
    viewport.x = 0.0F;
    viewport.y = 0.0F;
    viewport.width = (float)linkedRenderEngine->swapchain.extent.width;
    viewport.height = (float)linkedRenderEngine->swapchain.extent.height;
    viewport.minDepth = 0.0F;
    viewport.maxDepth = 1.0F;
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
    rasterizationStateCreateInfo.lineWidth = 1.0F;
    rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    if (linkedRenderEngine->device.physical_device.features.sampleRateShading != 0U) {
        multisampleStateCreateInfo.sampleShadingEnable = linkedRenderEngine->settings->msaaSamples >= VK_SAMPLE_COUNT_1_BIT ? VK_TRUE : VK_FALSE;
        multisampleStateCreateInfo.minSampleShading = 1.0F;
    }
    multisampleStateCreateInfo.rasterizationSamples = static_cast<VkSampleCountFlagBits>(linkedRenderEngine->settings->msaaSamples);
    VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
    pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
    pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
    pipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
    pipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS;
    VkPipelineColorBlendAttachmentState colorBlendAttachmentState{};
    colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachmentState.blendEnable = linkedRenderEngine->settings->rayTracing ? VK_FALSE : VK_TRUE;
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
    if (vkCreateGraphicsPipelines(linkedRenderEngine->device.device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS) {
        linkedRenderEngine->settings->logger.log(ILLUMINATION_ENGINE_LOG_LEVEL_ERROR, "Failed to create pipeline!");
    }
    deletionQueue.emplace_back([&] {
        vkDestroyPipeline(linkedRenderEngine->device.device, pipeline, nullptr);
    });
}

IEPipeline::~IEPipeline() {
    destroy();
}
#include "Pipeline.hpp"

#include "RenderEngine.hpp"
#include "Renderable/Vertex.hpp"
#include "Subpass.hpp"

void IE::Graphics::Pipeline::build(
  IE::Graphics::Subpass                      *t_subpass,
  const std::vector<std::shared_ptr<Shader>> &t_shaders
) {
    m_linkedRenderEngine = t_subpass->m_linkedRenderEngine;
    m_subpass            = t_subpass;

    // Build pipeline layout
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    std::vector<VkPushConstantRange>   pushConstantRanges;

    for (size_t i{}; i < 4; ++i)
        descriptorSetLayouts.push_back(IE::Graphics::DescriptorSet::getLayout(m_linkedRenderEngine, i, t_shaders));

    VkPipelineLayoutCreateInfo layoutCreateInfo{
      .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
      .pNext                  = nullptr,
      .flags                  = 0x0,
      .setLayoutCount         = static_cast<uint32_t>(descriptorSetLayouts.size()),
      .pSetLayouts            = descriptorSetLayouts.data(),
      .pushConstantRangeCount = static_cast<uint32_t>(pushConstantRanges.size()),
      .pPushConstantRanges    = pushConstantRanges.data()};

    VkResult layoutResult{
      vkCreatePipelineLayout(m_linkedRenderEngine->m_device.device, &layoutCreateInfo, nullptr, &m_layout)};
    // As they are no longer needed, the descriptor set layouts get destroyed
    for (auto &layout : descriptorSetLayouts)
        vkDestroyDescriptorSetLayout(m_linkedRenderEngine->m_device.device, layout, nullptr);
    if (layoutResult != VK_SUCCESS)
        m_linkedRenderEngine->getLogger().log(
          "Failed to create pipeline layout with error: " +
            IE::Graphics::RenderEngine::translateVkResultCodes(layoutResult),
          Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    else m_linkedRenderEngine->getLogger().log("Created Pipeline Layout");

    // Build pipeline cache
    VkPipelineCacheCreateInfo cacheCreateInfo{
      .sType           = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
      .pNext           = nullptr,
      .flags           = 0x0,
      .initialDataSize = 0x0,
      .pInitialData    = nullptr,
    };
    VkResult cacheResult{
      vkCreatePipelineCache(m_linkedRenderEngine->m_device.device, &cacheCreateInfo, nullptr, &m_cache)};
    if (cacheResult != VK_SUCCESS)
        m_linkedRenderEngine->getLogger().log(
          "Failed to create pipeline cache with error: " +
            IE::Graphics::RenderEngine::translateVkResultCodes(cacheResult),
          Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    else m_linkedRenderEngine->getLogger().log("Created Pipeline Cache");

    // Pipeline shader stage
    std::vector<VkSpecializationMapEntry> specializationMaps(0);
    std::vector<uint32_t>                 specializationData(0);

    VkSpecializationInfo specializationInfo{
      .mapEntryCount = 0x0,
      .pMapEntries   = specializationMaps.data(),
      .dataSize      = sizeof(uint32_t),
      .pData         = specializationData.data(),
    };

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages(t_shaders.size());
    VkPipelineShaderStageCreateInfo              shaderStageCreateInfo{
                   .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                   .pNext               = nullptr,
                   .flags               = 0x0,
                   .pName               = "main",
                   .pSpecializationInfo = &specializationInfo,
    };
    size_t i{0};
    std::generate_n(shaderStages.begin(), shaderStages.size(), [&]() {
        shaderStageCreateInfo.stage  = t_shaders[i]->m_stage;
        shaderStageCreateInfo.module = t_shaders[i++]->m_module;
        return shaderStageCreateInfo;
    });

    // Pipeline vertex input state
    std::vector<VkVertexInputBindingDescription> vertexBindingDescriptions{
      IE::Graphics::Vertex::getBindingDescription()};
    std::vector<VkVertexInputAttributeDescription> vertexAttributeDescription{
      IE::Graphics::Vertex::getAttributeDescriptions(),
    };

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{
      .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
      .pNext                           = nullptr,
      .flags                           = 0x0,
      .vertexBindingDescriptionCount   = static_cast<uint32_t>(vertexBindingDescriptions.size()),
      .pVertexBindingDescriptions      = vertexBindingDescriptions.data(),
      .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescription.size()),
      .pVertexAttributeDescriptions    = vertexAttributeDescription.data(),
    };

    // Pipeline vertex assembly state
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{
      .sType    = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
      .pNext    = nullptr,
      .flags    = 0x0,
      .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    };

    // Pipeline tessellation state
    VkPipelineTessellationStateCreateInfo tessellationStateCreateInfo{
      .sType              = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
      .pNext              = nullptr,
      .flags              = 0x0,
      .patchControlPoints = 0x0,
    };

    // Pipeline viewport state
    std::vector<VkViewport> viewports{
      {.x        = 0,
       .y        = 0,
       .width    = static_cast<float>(m_linkedRenderEngine->m_currentResolution[0]),
       .height   = static_cast<float>(m_linkedRenderEngine->m_currentResolution[1]),
       .minDepth = 0,
       .maxDepth = 1},
    };
    std::vector<VkRect2D> scissors{
      {
       {.x = 0, .y = 0},
       {.width  = static_cast<uint32_t>(m_linkedRenderEngine->m_currentResolution[0]),
         .height = static_cast<uint32_t>(m_linkedRenderEngine->m_currentResolution[1])},
       }
    };

    // Pipeline viewport state
    VkPipelineViewportStateCreateInfo viewportStateCreateInfo{
      .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
      .pNext         = nullptr,
      .flags         = 0x0,
      // Because viewport state is dynamic, viewports are ignored. They're included for completeness and debugging.
      .viewportCount = static_cast<uint32_t>(viewports.size()),
      .pViewports    = viewports.data(),
      // Because scissor state is dynamic scissors are ignored. They're included for completeness and debugging.
      .scissorCount  = static_cast<uint32_t>(scissors.size()),
      .pScissors     = scissors.data(),
    };

    // Pipeline rasterization state
    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{
      .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
      .pNext                   = nullptr,
      .flags                   = 0x0,
      .depthClampEnable        = VK_FALSE,
      .rasterizerDiscardEnable = VK_TRUE,
      .polygonMode             = VK_POLYGON_MODE_FILL,
      .cullMode                = VK_CULL_MODE_BACK_BIT,
      .frontFace               = VK_FRONT_FACE_COUNTER_CLOCKWISE,
      .depthBiasEnable         = VK_FALSE,
      .depthBiasConstantFactor = 0,
      .depthBiasClamp          = 0,
      .depthBiasSlopeFactor    = 0,
      .lineWidth               = 1,
    };

    // Pipeline multisample state
    VkSampleMask sampleMask{};

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{
      .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
      .pNext                 = nullptr,
      .flags                 = 0x0,
      .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
      .sampleShadingEnable   = VK_FALSE,
      .minSampleShading      = 0,
      .pSampleMask           = &sampleMask,
      .alphaToCoverageEnable = VK_FALSE,
      .alphaToOneEnable      = VK_FALSE,
    };

    // Pipeline depth / stencil state
    VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{
      .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
      .pNext                 = nullptr,
      .flags                 = 0x0,
      .depthTestEnable       = VK_TRUE,
      .depthWriteEnable      = VK_TRUE,
      .depthCompareOp        = VK_COMPARE_OP_GREATER,
      .depthBoundsTestEnable = VK_FALSE,
      .stencilTestEnable     = VK_FALSE,
      .front =
        {
                .failOp      = VK_STENCIL_OP_KEEP,
                .passOp      = VK_STENCIL_OP_REPLACE,
                .depthFailOp = VK_STENCIL_OP_KEEP,
                .compareOp   = VK_COMPARE_OP_GREATER,
                .compareMask = UINT32_MAX,
                .writeMask   = UINT32_MAX,
                .reference   = 0x0,
                },
      .back =
        {
                .failOp      = VK_STENCIL_OP_KEEP,
                .passOp      = VK_STENCIL_OP_REPLACE,
                .depthFailOp = VK_STENCIL_OP_KEEP,
                .compareOp   = VK_COMPARE_OP_GREATER,
                .compareMask = UINT32_MAX,
                .writeMask   = UINT32_MAX,
                .reference   = 0x0,
                },
      .minDepthBounds = 0,
      .maxDepthBounds = 1,
    };

    // Pipeline color blend state
    std::vector<VkPipelineColorBlendAttachmentState> attachments(0);

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{
      .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
      .pNext           = nullptr,
      .flags           = 0x0,
      .logicOpEnable   = VK_FALSE,
      .logicOp         = VK_LOGIC_OP_NO_OP,
      .attachmentCount = static_cast<uint32_t>(attachments.size()),
      .pAttachments    = attachments.data(),
      .blendConstants  = {0.0, 0.0, 0.0, 0.0},
    };

    // Pipeline dynamic state
    std::vector<VkDynamicState> dynamicStates{VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo{
      .sType             = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
      .pNext             = nullptr,
      .flags             = 0x0,
      .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
      .pDynamicStates    = dynamicStates.data(),
    };

    // Pipeline assembly
    uint32_t subpassNumber{0};
    for (; subpassNumber < m_subpass->m_renderPass->m_subpasses.size(); ++subpassNumber)
        if (m_subpass->m_renderPass->m_subpasses[subpassNumber].get() == m_subpass) break;

    VkGraphicsPipelineCreateInfo pipelineCreateInfo{
      .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext               = nullptr,
      .flags               = 0x0,
      .stageCount          = static_cast<uint32_t>(shaderStages.size()),
      .pStages             = shaderStages.data(),
      .pVertexInputState   = &vertexInputStateCreateInfo,
      .pInputAssemblyState = &inputAssemblyStateCreateInfo,
      .pTessellationState  = &tessellationStateCreateInfo,
      .pViewportState      = &viewportStateCreateInfo,
      .pRasterizationState = &rasterizationStateCreateInfo,
      .pMultisampleState   = &multisampleStateCreateInfo,
      .pDepthStencilState  = &depthStencilStateCreateInfo,
      .pColorBlendState    = &colorBlendStateCreateInfo,
      .pDynamicState       = &dynamicStateCreateInfo,
      .layout              = m_layout,
      .renderPass          = m_subpass->m_renderPass->m_renderPass,
      .subpass             = subpassNumber,
      .basePipelineHandle  = VK_NULL_HANDLE,
    };

    VkResult result{vkCreateGraphicsPipelines(
      m_linkedRenderEngine->m_device.device,
      m_cache,
      1,
      &pipelineCreateInfo,
      nullptr,
      &m_pipeline
    )};
    if (result != VK_SUCCESS)
        m_linkedRenderEngine->getLogger().log(
          "Failed to create graphics pipeline! Error: " +
            IE::Graphics::RenderEngine::makeErrorMessage(
              IE::Graphics::RenderEngine::translateVkResultCodes(result),
              "build",
              __FILE__,
              __LINE__,
              "https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkGraphicsPipelineCreateInfo.html"
            ),
          Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    else m_linkedRenderEngine->getLogger().log("Created Pipeline");
}

void IE::Graphics::Pipeline::destroy() {
    if (m_layout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(m_linkedRenderEngine->m_device.device, m_layout, nullptr);
        m_layout = VK_NULL_HANDLE;
    }
    if (m_cache != VK_NULL_HANDLE) {
        vkDestroyPipelineCache(m_linkedRenderEngine->m_device.device, m_cache, nullptr);
        m_cache = VK_NULL_HANDLE;
    }
    if (m_pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(m_linkedRenderEngine->m_device.device, m_pipeline, nullptr);
        m_pipeline = VK_NULL_HANDLE;
    }
}

IE::Graphics::Pipeline::~Pipeline() {
    destroy();
}

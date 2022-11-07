#include "Pipeline.hpp"

#include "Renderable/Vertex.hpp"
#include "RenderEngine.hpp"
#include "RenderPass.hpp"
#include "Subpass.hpp"

void IE::Graphics::Pipeline::build() {
    // Pipeline shader stage
    std::vector<VkSpecializationMapEntry> specializationMaps(0);
    std::vector<uint32_t>                 specializationData(0);

    VkSpecializationInfo specializationInfo{
      .mapEntryCount = 0x0,
      .pMapEntries   = specializationMaps.data(),
      .dataSize      = sizeof(uint32_t),
      .pData         = specializationData.data(),
    };

    VkPipelineShaderStageCreateInfo shaderStageCreateInfo{
      .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .pNext               = nullptr,
      .flags               = 0x0,
      .stage               = m_subpass->shader.stage,
      .module              = m_subpass->shader.module,
      .pName               = "main",
      .pSpecializationInfo = &specializationInfo,
    };

    // Pipeline vertex input state
    std::vector<VkVertexInputBindingDescription> vertexBindingDescriptions{
      IE::Graphics::Vertex::getBindingDescription()};
    std::vector<VkVertexInputAttributeDescription> vertexAttributeDescription{
      IE::Graphics::Vertex::getAttributeDescriptions(),
    };

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{
      .sType                           = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
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
      {.x = 0, .y = 0, .width = 800, .height = 600, .minDepth = 0, .maxDepth = 1},
    };
    std::vector<VkRect2D> scissors{
      {
       {.x = 0, .y = 0},
       {.width = 800, .height = 600},
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
    VkGraphicsPipelineCreateInfo pipelineCreateInfo{
      .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
      .pNext               = nullptr,
      .flags               = 0x0,
      .stageCount          = 0x0,
      .pStages             = &shaderStageCreateInfo,
      .pVertexInputState   = &vertexInputStateCreateInfo,
      .pInputAssemblyState = &inputAssemblyStateCreateInfo,
      .pTessellationState  = &tessellationStateCreateInfo,
      .pViewportState      = &viewportStateCreateInfo,
      .pRasterizationState = &rasterizationStateCreateInfo,
      .pMultisampleState   = &multisampleStateCreateInfo,
      .pDepthStencilState  = &depthStencilStateCreateInfo,
      .pColorBlendState    = &colorBlendStateCreateInfo,
      .pDynamicState       = &dynamicStateCreateInfo,
      .layout              = VK_NULL_HANDLE,
      .renderPass          = m_subpass->m_owningRenderPass->m_renderPass,
      .subpass             = 0x0,
      .basePipelineHandle  = VK_NULL_HANDLE,
    };

    VkResult result{vkCreateGraphicsPipelines(
      m_subpass->m_linkedRenderEngine->m_device.device,
      m_cache,
      1,
      &pipelineCreateInfo,
      nullptr,
      &m_pipeline
    )};
    if (result != VK_SUCCESS)
        m_subpass->m_linkedRenderEngine->getLogger().log(
          "Failed to create graphics pipeline! Error: " +
          IE::Graphics::RenderEngine::makeErrorMessage(
            IE::Graphics::RenderEngine::translateVkResultCodes(result),
            "build",
            __FILE__,
            __LINE__,
            "https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkGraphicsPipelineCreateInfo.html"
          )
        );
    else m_subpass->m_linkedRenderEngine->getLogger().log("Created Pipeline!");
}

IE::Graphics::Pipeline::Pipeline(IE::Graphics::Subpass *t_subpass) : m_subpass(t_subpass) {
}

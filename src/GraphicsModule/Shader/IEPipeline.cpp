/* Include this file's header. */
#include "IEPipeline.hpp"

/* Include dependencies within this module. */
#include "IEDescriptorSet.hpp"
#include "IERenderEngine.hpp"
#include "GraphicsModule/Renderable/IEVertex.hpp"

/* Include dependencies from Core. */
#include "Core/LogModule/IELogger.hpp"

#include <memory>


void IEPipeline::setAPI(const API &API) {
	if (API.name == IE_RENDER_ENGINE_API_NAME_OPENGL) {
		_create = &IEPipeline::_openglCreate;
	} else if (API.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
		_create = &IEPipeline::_vulkanCreate;
	}
}

void IEPipeline::destroy() {
	for (const std::function<void()> &function: deletionQueue) {
		function();
	}
	deletionQueue.clear();
}

std::function<void(IEPipeline &, IERenderEngine *, IEPipeline::CreateInfo *)> IEPipeline::_create = nullptr;

void IEPipeline::create(IERenderEngine *engineLink, IEPipeline::CreateInfo *createInfo) {
	linkedRenderEngine = engineLink;
	_create(*this, engineLink, createInfo);
}

void IEPipeline::_openglCreate(IERenderEngine *engineLink, IEPipeline::CreateInfo *createInfo) {
	programID = glCreateProgram();
	for (std::shared_ptr<IEShader> &shader: createInfo->shaders) {
		glAttachShader(programID, shader->shaderID);
	}
	glLinkProgram(programID);

	GLint result;
	GLint infoLogLength;
	glGetProgramiv(programID, GL_LINK_STATUS, &result);
	glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
	if (infoLogLength > 0) {
		std::vector<char> programErrorMessage(infoLogLength + 1);
		glGetProgramInfoLog(programID, infoLogLength, nullptr, &programErrorMessage[0]);
		printf("%s\n", &programErrorMessage[0]);
	}
}

void IEPipeline::_vulkanCreate(IERenderEngine *engineLink, IEPipeline::CreateInfo *createInfo) {
	linkedRenderEngine = engineLink;
	createdWith = *createInfo;
	// Create pipelineLayout
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{
			.sType=VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount=1,
			.pSetLayouts=&createdWith.descriptorSet.lock()->descriptorSetLayout,
	};
	if (vkCreatePipelineLayout(linkedRenderEngine->device.m_device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        linkedRenderEngine->settings->m_logger.log(
          "Failed to create pipeline layout!",
          ILLUMINATION_ENGINE_LOG_LEVEL_WARN
        );
	}

	deletionQueue.emplace_back([&] {
		vkDestroyPipelineLayout(linkedRenderEngine->device.m_device, pipelineLayout, nullptr);
	});

	// Prepare shaders
	std::array<VkShaderStageFlagBits, 2> shaderStages{VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT};
	std::vector<VkPipelineShaderStageCreateInfo> shaders{};
	for (uint32_t i = 0; i < createdWith.shaders.size(); i++) {
		VkPipelineShaderStageCreateInfo shaderStageInfo{
				.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
				.stage=shaderStages[i],
				.module=createdWith.shaders[i]->module,
				.pName="main",
		};
		shaders.push_back(shaderStageInfo);
	}

	// Create graphics pipeline
	VkVertexInputBindingDescription bindingDescription = IEVertex::getBindingDescription();
	std::array<VkVertexInputAttributeDescription, 6> attributeDescriptions = IEVertex::getAttributeDescriptions();
	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{
			.sType=VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.vertexBindingDescriptionCount=1,
			.pVertexBindingDescriptions=&bindingDescription,
			.vertexAttributeDescriptionCount=static_cast<uint32_t>(attributeDescriptions.size()),
			.pVertexAttributeDescriptions=attributeDescriptions.data(),
	};
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo{
			.sType=VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology=VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable=VK_FALSE,
	};
	VkViewport viewport{
			.x=0.0F,
			.y=0.0F,
			.width=(float) linkedRenderEngine->swapchain.extent.width,
			.height=(float) linkedRenderEngine->swapchain.extent.height,
			.minDepth=0.0F,
			.maxDepth=1.0F,
	};
	VkRect2D scissor{
			.offset={0, 0},
			.extent=linkedRenderEngine->swapchain.extent,
	};
	VkPipelineViewportStateCreateInfo viewportStateCreateInfo{
			.sType=VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount=1,
			.pViewports=&viewport,
			.scissorCount=1,
			.pScissors=&scissor,
	};
	VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{
			.sType=VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable=VK_FALSE,
			.rasterizerDiscardEnable=VK_FALSE,
			.polygonMode=VK_POLYGON_MODE_FILL, //Controls fill mode (e.g. wireframe mode,
			.cullMode=VK_CULL_MODE_BACK_BIT,
			.frontFace=VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable=VK_FALSE,
			.lineWidth=1.0F,
	};
	VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{
			.sType=VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples=static_cast<VkSampleCountFlagBits>(linkedRenderEngine->settings->msaaSamples),
	};
	if (linkedRenderEngine->device.physical_device.features.sampleRateShading != 0U) {
		multisampleStateCreateInfo.sampleShadingEnable = linkedRenderEngine->settings->msaaSamples >= VK_SAMPLE_COUNT_1_BIT ? VK_TRUE : VK_FALSE;
		multisampleStateCreateInfo.minSampleShading = 1.0F;
	}
	VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{
			.sType=VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable=VK_TRUE,
			.depthWriteEnable=VK_TRUE,
			.depthCompareOp=VK_COMPARE_OP_LESS_OR_EQUAL,
			.depthBoundsTestEnable=VK_FALSE,
			.stencilTestEnable=VK_FALSE,
			.back={
					.compareOp=VK_COMPARE_OP_ALWAYS,
			}
	};
	VkPipelineColorBlendAttachmentState colorBlendAttachmentState{
			.blendEnable=linkedRenderEngine->settings->rayTracing ? VK_FALSE : VK_TRUE,
			.srcColorBlendFactor=VK_BLEND_FACTOR_SRC_ALPHA,
			.dstColorBlendFactor=VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
			.colorBlendOp=VK_BLEND_OP_ADD,
			.srcAlphaBlendFactor=VK_BLEND_FACTOR_ONE,
			.dstAlphaBlendFactor=VK_BLEND_FACTOR_ZERO,
			.alphaBlendOp=VK_BLEND_OP_ADD,
			.colorWriteMask=VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
	};
	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{
			.sType=VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable=VK_FALSE,
			.attachmentCount=1,
			.pAttachments=&colorBlendAttachmentState,
	};
	VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{
			.sType=VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount=2,
			.pDynamicStates=dynamicStates,
	};
	VkGraphicsPipelineCreateInfo pipelineCreateInfo{
			.sType=VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.stageCount=2,
			.pStages=shaders.data(),
			.pVertexInputState=&vertexInputStateCreateInfo,
			.pInputAssemblyState=&inputAssemblyStateCreateInfo,
			.pViewportState=&viewportStateCreateInfo,
			.pRasterizationState=&rasterizationStateCreateInfo,
			.pMultisampleState=&multisampleStateCreateInfo,
			.pDepthStencilState=&pipelineDepthStencilStateCreateInfo,
			.pColorBlendState=&colorBlendStateCreateInfo,
			.pDynamicState=&pipelineDynamicStateCreateInfo,
			.layout=pipelineLayout,
			.renderPass=createdWith.renderPass.lock()->renderPass,
			.basePipelineHandle=VK_NULL_HANDLE
	};
	if (vkCreateGraphicsPipelines(linkedRenderEngine->device.m_device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS) {
        linkedRenderEngine->settings->m_logger.log(
          "Failed to create pipeline!",
          ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
	}
	deletionQueue.emplace_back([&] {
		vkDestroyPipeline(linkedRenderEngine->device.m_device, pipeline, nullptr);
	});
}


IEPipeline::~IEPipeline() {
	destroy();
}

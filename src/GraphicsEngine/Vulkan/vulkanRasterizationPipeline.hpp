#pragma once

#include "vulkanGraphicsEngineLink.hpp"

#include <deque>
#include <functional>

enum DescriptorAttachmentType {
    BUFFER = 0,
    IMAGE = 1
};

class RasterizationPipelineManager {
public:
    VkPipelineLayout pipelineLayout{};
    VkDescriptorSetLayout descriptorSetLayout{};
    VkPipeline pipeline{};
    VkDescriptorPool descriptorPool{};
    VkDescriptorSet descriptorSet{};


    void destroy() {
        for (const std::function<void()>& function : deletionQueue) { function(); }
        deletionQueue.clear();
    }

    void setup(VulkanGraphicsEngineLink *engineLink, const std::vector<VkDescriptorType>& setupDescriptorTypes, const std::vector<VkShaderStageFlagBits>& setupShaderFlags, uint32_t setupSwapchainImageCount, VkRenderPass renderPass, std::vector<std::vector<char>> shaderData) {
        linkedRenderEngine = engineLink;
        //create descriptor layout
        if (setupDescriptorTypes.size() != setupShaderFlags.size()) { throw std::runtime_error("number of descriptor types does not equal number of shader flags!"); }
        swapchainImageCount = setupSwapchainImageCount;
        descriptorTypes = setupDescriptorTypes;
        descriptorSetLayoutBindings.clear();
        descriptorPoolSizes.clear();
        descriptorSetLayoutBindings.reserve(setupDescriptorTypes.size());
        descriptorPoolSizes.reserve(setupDescriptorTypes.size());
        VkDescriptorSetLayoutBinding descriptorSetLayoutBinding{};
        descriptorSetLayoutBinding.descriptorCount = 1;
        VkDescriptorPoolSize descriptorPoolSize{};
        descriptorPoolSize.descriptorCount = swapchainImageCount;
        for (unsigned long i = 0; i < setupDescriptorTypes.size(); i++) {
            descriptorSetLayoutBinding.descriptorType = setupDescriptorTypes[i];
            descriptorSetLayoutBinding.stageFlags = setupShaderFlags[i];
            descriptorSetLayoutBinding.binding = i;
            descriptorSetLayoutBindings.push_back(descriptorSetLayoutBinding);
            descriptorPoolSize.type = setupDescriptorTypes[i];
            descriptorPoolSizes.push_back(descriptorPoolSize);
        }
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
        descriptorSetLayoutCreateInfo.bindingCount = static_cast<uint32_t>(descriptorSetLayoutBindings.size());
        descriptorSetLayoutCreateInfo.pBindings = descriptorSetLayoutBindings.data();
        if (vkCreateDescriptorSetLayout(linkedRenderEngine->device->device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) { throw std::runtime_error("failed to create descriptor set layout!"); }
        deletionQueue.emplace_front([&]{ vkDestroyDescriptorSetLayout(linkedRenderEngine->device->device, descriptorSetLayout, nullptr); descriptorSetLayout = VK_NULL_HANDLE; });
        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
        descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCreateInfo.poolSizeCount = static_cast<uint32_t>(descriptorPoolSizes.size());
        descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
        descriptorPoolCreateInfo.maxSets = swapchainImageCount;
        if (vkCreateDescriptorPool(linkedRenderEngine->device->device, &descriptorPoolCreateInfo, nullptr, &descriptorPool) != VK_SUCCESS) { throw std::runtime_error("failed to create descriptor pool!"); }
        deletionQueue.emplace_front([&]{ vkDestroyDescriptorPool(linkedRenderEngine->device->device, descriptorPool, nullptr); descriptorPool = VK_NULL_HANDLE; });
        //Create pipelineLayout
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
        if (vkCreatePipelineLayout(linkedRenderEngine->device->device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) { throw std::runtime_error("failed to create pipeline layout!"); }
        deletionQueue.emplace_front([&]{ vkDestroyPipelineLayout(linkedRenderEngine->device->device, pipelineLayout, nullptr); pipelineLayout = VK_NULL_HANDLE; });
        //prepare shaders
        std::vector<VkPipelineShaderStageCreateInfo> shaders{};
        for (unsigned int i = 0; i < shaderData.size(); i++) {
            VkShaderModule shaderModule;
            VkShaderModuleCreateInfo shaderModuleCreateInfo{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
            shaderModuleCreateInfo.codeSize = shaderData[i].size();
            shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t *>(shaderData[i].data());
            if (vkCreateShaderModule(linkedRenderEngine->device->device, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) { throw std::runtime_error("failed to create shader module!"); }
            VkPipelineShaderStageCreateInfo shaderStageInfo{VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
            shaderStageInfo.module = shaderModule;
            shaderStageInfo.pName = "main";
            shaderStageInfo.stage = i % 2 ? VK_SHADER_STAGE_FRAGMENT_BIT : VK_SHADER_STAGE_VERTEX_BIT;
            shaders.push_back(shaderStageInfo);
        }
        //create graphics pipeline
        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        VkVertexInputBindingDescription bindingDescription = Vertex::getBindingDescription();
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = Vertex::getAttributeDescriptions();
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        VkPipelineInputAssemblyStateCreateInfo  inputAssemblyStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
        inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
        VkViewport viewport{};
        viewport.x = 0.f;
        viewport.y = 0.f;
        viewport.width = (float)linkedRenderEngine->swapchain->extent.width;
        viewport.height = (float)linkedRenderEngine->swapchain->extent.height;
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = linkedRenderEngine->swapchain->extent;
        VkPipelineViewportStateCreateInfo viewportStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
        viewportStateCreateInfo.viewportCount = 1;
        viewportStateCreateInfo.pViewports = &viewport;
        viewportStateCreateInfo.scissorCount = 1;
        viewportStateCreateInfo.pScissors = &scissor;
        VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
        rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; //Controls fill mode (e.g. wireframe mode)
        rasterizationStateCreateInfo.lineWidth = 1.f;
        rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
        VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
        multisampleStateCreateInfo.sampleShadingEnable = linkedRenderEngine->settings->msaaSamples == VK_SAMPLE_COUNT_1_BIT ? VK_FALSE : VK_TRUE;
        multisampleStateCreateInfo.rasterizationSamples = linkedRenderEngine->settings->msaaSamples;
        VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
        pipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
        pipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
        pipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        pipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
        pipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
        VkPipelineColorBlendAttachmentState colorBlendAttachmentState{}; //Alpha blending is done here
        colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachmentState.blendEnable = VK_TRUE;
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
        pipelineCreateInfo.renderPass = renderPass;
        pipelineCreateInfo.subpass = 0;
        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        if (vkCreateGraphicsPipelines(linkedRenderEngine->device->device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline) != VK_SUCCESS) { throw std::runtime_error("failed to create graphics pipeline!"); }
        for (VkPipelineShaderStageCreateInfo shader : shaders) { vkDestroyShaderModule(linkedRenderEngine->device->device, shader.module, nullptr); }
        deletionQueue.emplace_front([&]{ vkDestroyPipeline(linkedRenderEngine->device->device, pipeline, nullptr); pipeline = VK_NULL_HANDLE; });
    }

    void createDescriptorSet(const std::vector<BufferManager>& buffers, const std::vector<ImageManager>& images, const std::vector<bool>& indices) {
        if (buffers.size() + images.size() != indices.size()) { throw std::runtime_error("number of indices does not equal number of images plus number of buffers!"); }
        VkDescriptorSetAllocateInfo descriptorSetAllocateInfo{VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
        descriptorSetAllocateInfo.descriptorPool = descriptorPool;
        descriptorSetAllocateInfo.pSetLayouts = &descriptorSetLayout;
        descriptorSetAllocateInfo.descriptorSetCount = 1;
        vkAllocateDescriptorSets(linkedRenderEngine->device->device, &descriptorSetAllocateInfo, &descriptorSet);
        int bufferCounter{}, imageCounter{};
        std::vector<VkWriteDescriptorSet> descriptorWrites{indices.size()};
        std::vector<VkDescriptorBufferInfo> descriptorBuffers{};
        descriptorBuffers.reserve(buffers.size());
        std::vector<VkDescriptorImageInfo> descriptorImages{};
        descriptorImages.reserve(images.size());
        for (unsigned long i = 0; i < indices.size(); i++) {
            descriptorWrites[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[i].dstSet = descriptorSet;
            descriptorWrites[i].dstBinding = i;
            descriptorWrites[i].dstArrayElement = 0;
            descriptorWrites[i].descriptorType = descriptorTypes[i];
            descriptorWrites[i].descriptorCount = 1;
            if (!indices[i]) {
                VkDescriptorBufferInfo descriptorBufferInfo{};
                descriptorBufferInfo.buffer = buffers[bufferCounter].buffer;
                descriptorBufferInfo.offset = 0;
                descriptorBufferInfo.range = buffers[bufferCounter].bufferSize;
                descriptorBuffers.push_back(descriptorBufferInfo);
                descriptorWrites[i].pBufferInfo = &descriptorBuffers[bufferCounter];
                bufferCounter++;
            } else {
                VkDescriptorImageInfo descriptorImageInfo{};
                descriptorImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                descriptorImageInfo.sampler = images[imageCounter].sampler;
                descriptorImageInfo.imageView = images[imageCounter].view;
                descriptorImages.push_back(descriptorImageInfo);
                descriptorWrites[i].pImageInfo = &descriptorImages[imageCounter];
                imageCounter++;
            }
        }
        vkUpdateDescriptorSets(linkedRenderEngine->device->device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

private:
    VulkanGraphicsEngineLink *linkedRenderEngine{};
    std::deque<std::function<void()>> deletionQueue{};
    std::vector<VkDescriptorType> descriptorTypes{};
    std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings{};
    std::vector<VkDescriptorPoolSize> descriptorPoolSizes{};
    uint32_t swapchainImageCount{};
};
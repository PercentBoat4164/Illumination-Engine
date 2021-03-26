#pragma once

#include <functional>
#include <deque>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vk-bootstrap/src/VkBootstrap.h>


class VulkanRenderEngine {
public:
    explicit VulkanRenderEngine(Settings &initialSettings = *new Settings{}, GLFWwindow *attachWindow = nullptr) {
        settings = initialSettings;
        renderEngineLink.device = &device;
        renderEngineLink.settings = &settings;
        renderEngineLink.commandPool = &commandPool;
        initializeVulkan(attachWindow);
    }

    void updateSettings(Settings newSettings, bool updateAll) {
        if (!settings.fullscreen & newSettings.fullscreen) { glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, newSettings.resolution[0], newSettings.resolution[0], newSettings.refreshRate); }
        else if (settings.fullscreen & !newSettings.fullscreen) { glfwSetWindowMonitor(window, nullptr, 0, 0, 800, 600, newSettings.refreshRate); }
        else { glfwSetWindowSize(window, newSettings.resolution[0], newSettings.resolution[1]); }
        settings = newSettings;
        if (updateAll) { createSwapchain(true); }
    }

    bool update() {
        if (assets.empty()) { return glfwWindowShouldClose(window) != 1; }
        vkWaitForFences(device.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        uint32_t imageIndex = 0;
        VkResult result = vkAcquireNextImageKHR(device.device, swapchain.swapchain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            createSwapchain();
            return glfwWindowShouldClose(window) != 1;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) { throw std::runtime_error("failed to acquire swapchain image!"); }
        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) { vkWaitForFences(device.device, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX); }
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];
        //update state of frame
        VkCommandBufferBeginInfo commandBufferBeginInfo{};
        commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        VkCommandBufferResetFlags commandBufferResetFlags{};
        vkResetCommandBuffer(commandBuffers[(imageIndex + (swapchain.image_count - 1)) % swapchain.image_count], commandBufferResetFlags);
        if (vkBeginCommandBuffer(commandBuffers[imageIndex], &commandBufferBeginInfo) != VK_SUCCESS) { throw std::runtime_error("failed to begin recording command buffer!"); }
        VkDeviceSize offsets[] = {0};
        std::vector<VkClearValue> clearValues{static_cast<size_t>(settings.msaaSamples == VK_SAMPLE_COUNT_1_BIT ? 2 : 3)};
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        if (settings.msaaSamples != VK_SAMPLE_COUNT_1_BIT) { clearValues[2].color = {0.0f, 0.0f, 0.0f, 1.0f}; }
        //Setup render pass to clear
        VkRenderPassBeginInfo renderPassBeginInfo{};
        renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassBeginInfo.renderArea.offset = {0, 0};
        renderPassBeginInfo.renderArea.extent = swapchain.extent;
        renderPassBeginInfo.clearValueCount = clearValues.size();
        renderPassBeginInfo.pClearValues = clearValues.data();
        VkViewport viewport{};
        viewport.x = 0.f;
        viewport.y = 0.f;
        viewport.width = (float)swapchain.extent.width;
        viewport.height = (float)swapchain.extent.height;
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapchain.extent;
        vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
        vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);
        for (Asset *asset : assets) {
            //update uniform buffer
            asset->updateUbo();
            //update textures
            //update mesh
            //update shaders?
            //record command buffers
            renderPassBeginInfo.renderPass = asset->renderPass;
            renderPassBeginInfo.framebuffer = asset->framebuffers[imageIndex];
            vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, asset->graphicsPipeline);
            vkCmdBindVertexBuffers(commandBuffers[imageIndex], 0, 1, &asset->vertexBuffer.buffer, offsets);
            vkCmdBindIndexBuffer(commandBuffers[imageIndex], asset->indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
            vkCmdBindDescriptorSets(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &asset->descriptorSet, 0, nullptr);
            vkCmdDrawIndexed(commandBuffers[imageIndex], asset->indices.size(), 1, 0, 0, 0);
            vkCmdEndRenderPass(commandBuffers[imageIndex]);
            //Set render pass to not clear as to only clear contents of previous frame
            renderPassBeginInfo.clearValueCount = 0;
            renderPassBeginInfo.pClearValues = nullptr;
        }
        if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) { throw std::runtime_error("failed to record command buffer!"); }
        //Submit
        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        vkResetFences(device.device, 1, &inFlightFences[currentFrame]);
        if (vkQueueSubmit(device.get_queue(vkb::QueueType::graphics).value(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) { throw std::runtime_error("failed to submit draw command buffer!"); }
        //Present
        VkSwapchainKHR swapchains[] = {swapchain.swapchain};
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapchains;
        presentInfo.pImageIndices = &imageIndex;
        result = vkQueuePresentKHR(device.get_queue(vkb::QueueType::present).value(), &presentInfo);
        vkQueueWaitIdle(device.get_queue(vkb::QueueType::present).value());
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            createSwapchain();
        } else if (result != VK_SUCCESS) { throw std::runtime_error("failed to present swapchain image!"); }
        currentFrame = (currentFrame + 1) % settings.MAX_FRAMES_IN_FLIGHT;
        return glfwWindowShouldClose(window) != 1;
    }

    void uploadAsset(Asset *asset) {
        //upload mesh
        asset->vertexBuffer.setEngineLink(renderEngineLink);
        memcpy(asset->vertexBuffer.create(sizeof(asset->vertices[0]) * asset->vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), asset->vertices.data(), sizeof(asset->vertices[0]) * asset->vertices.size());
        asset->deletionQueue.emplace_front([&](Asset thisAsset){ thisAsset.vertexBuffer.destroy(); });
        asset->indexBuffer.setEngineLink(renderEngineLink);
        memcpy(asset->indexBuffer.create(sizeof(asset->indices[0]) * asset->indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), asset->indices.data(), sizeof(asset->indices[0]) * asset->indices.size());
        asset->deletionQueue.emplace_front([&](Asset thisAsset){ thisAsset.indexBuffer.destroy(); });
        //upload textures
        stagingBuffer.setEngineLink(renderEngineLink);
        memcpy(stagingBuffer.create(asset->width * asset->height * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), asset->textures[0], asset->width * asset->height * 4);
        asset->textureImages.resize(1);
        asset->textureImages[0].setEngineLink(renderEngineLink);
        asset->textureImages[0].create(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 1, asset->width, asset->height, false, true, &stagingBuffer);
        asset->deletionQueue.emplace_front([&](const Asset& thisAsset){ for (AllocatedImage textureImage : thisAsset.textureImages) { textureImage.destroy(); } });
        //prepare data for shader
        //build uniform buffers
        asset->uniformBuffer.setEngineLink(renderEngineLink);
        memcpy(asset->uniformBuffer.create(sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT), &asset->ubo, sizeof(UniformBufferObject));
        asset->deletionQueue.emplace_front([&](Asset thisAsset){ thisAsset.uniformBuffer.destroy(); });
        //build descriptor sets
        VkDescriptorPoolSize uboDescriptorPoolSize{};
        uboDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboDescriptorPoolSize.descriptorCount = swapchain.image_count;
        //Iterate for every image needed by the shader
        VkDescriptorPoolSize imageDescriptorPoolSize{};
        imageDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imageDescriptorPoolSize.descriptorCount = swapchain.image_count;
        //stop iterating
        VkDescriptorPoolCreateInfo descriptorPoolCreateInfo{};
        std::vector<VkDescriptorPoolSize> descriptorPoolSizes{uboDescriptorPoolSize, imageDescriptorPoolSize};
        descriptorPoolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptorPoolCreateInfo.poolSizeCount = descriptorPoolSizes.size();
        descriptorPoolCreateInfo.pPoolSizes = descriptorPoolSizes.data();
        descriptorPoolCreateInfo.maxSets = swapchain.image_count;
        vkCreateDescriptorPool(device.device, &descriptorPoolCreateInfo, nullptr, &asset->descriptorPool);
        asset->deletionQueue.emplace_front([&](const Asset& thisAsset){ vkDestroyDescriptorPool(device.device, thisAsset.descriptorPool, nullptr); });
        std::vector<VkDescriptorSetLayout> layouts(swapchain.image_count, descriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = asset->descriptorPool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = layouts.data();
        if (vkAllocateDescriptorSets(device.device, &allocInfo, &asset->descriptorSet) != VK_SUCCESS) { throw std::runtime_error("failed to allocate descriptor sets!"); }
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = asset->uniformBuffer.buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);
        std::vector<VkWriteDescriptorSet> descriptorWrites{2};
        descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[0].dstSet = asset->descriptorSet;
        descriptorWrites[0].dstBinding = 0;
        descriptorWrites[0].dstArrayElement = 0;
        descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrites[0].descriptorCount = 1;
        descriptorWrites[0].pBufferInfo = &bufferInfo;
        //Iterate this part for the number of textures used
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = asset->textureImages[0].view;
        imageInfo.sampler = asset->textureImages[0].sampler;
        descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrites[1].dstSet = asset->descriptorSet;
        descriptorWrites[1].dstBinding = 1;
        descriptorWrites[1].dstArrayElement = 0;
        descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptorWrites[1].descriptorCount = 1;
        descriptorWrites[1].pImageInfo = &imageInfo;
        //End the iterations here
        vkUpdateDescriptorSets(device.device, descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
        //create render pass for this asset
        VkAttachmentDescription colorAttachmentDescription{};
        colorAttachmentDescription.format = swapchain.image_format;
        colorAttachmentDescription.samples = settings.msaaSamples;
        colorAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentDescription.finalLayout = settings.msaaSamples == VK_SAMPLE_COUNT_1_BIT ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkAttachmentDescription depthAttachmentDescription{};
        depthAttachmentDescription.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        depthAttachmentDescription.samples = settings.msaaSamples;
        depthAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        VkAttachmentDescription colorResolveAttachmentDescription{};
        colorResolveAttachmentDescription.format = swapchain.image_format;
        colorResolveAttachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
        colorResolveAttachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorResolveAttachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorResolveAttachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorResolveAttachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorResolveAttachmentDescription.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorResolveAttachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        VkAttachmentReference colorAttachmentReference{};
        colorAttachmentReference.attachment = 0;
        colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkAttachmentReference depthAttachmentReference{};
        depthAttachmentReference.attachment = 1;
        depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        VkAttachmentReference colorResolveAttachmentReference{};
        colorResolveAttachmentReference.attachment = 2;
        colorResolveAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkSubpassDescription subpassDescription{};
        subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescription.colorAttachmentCount = 1;
        subpassDescription.pColorAttachments = &colorAttachmentReference;
        subpassDescription.pDepthStencilAttachment = &depthAttachmentReference;
        subpassDescription.pResolveAttachments = settings.msaaSamples == VK_SAMPLE_COUNT_1_BIT ? nullptr : &colorResolveAttachmentReference;
        VkSubpassDependency subpassDependency{};
        subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependency.dstSubpass = 0;
        subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpassDependency.srcAccessMask = 0;
        subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        std::vector<VkAttachmentDescription> attachmentDescriptions{};
        if (settings.msaaSamples != VK_SAMPLE_COUNT_1_BIT) { attachmentDescriptions = {colorAttachmentDescription, depthAttachmentDescription, colorResolveAttachmentDescription}; } else { attachmentDescriptions = {colorAttachmentDescription, depthAttachmentDescription}; }
        VkRenderPassCreateInfo renderPassCreateInfo{};
        renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassCreateInfo.attachmentCount = attachmentDescriptions.size();
        renderPassCreateInfo.pAttachments = attachmentDescriptions.data();
        renderPassCreateInfo.subpassCount = 1;
        renderPassCreateInfo.pSubpasses = &subpassDescription;
        renderPassCreateInfo.dependencyCount = 1;
        renderPassCreateInfo.pDependencies = &subpassDependency;
        if (vkCreateRenderPass(device.device, &renderPassCreateInfo, nullptr, &asset->renderPass) != VK_SUCCESS) { throw std::runtime_error("failed to create render pass!"); }
        asset->deletionQueue.emplace_front([&](const Asset& thisAsset){ vkDestroyRenderPass(device.device, thisAsset.renderPass, nullptr); });
        //prepare shaders
        std::vector<VkPipelineShaderStageCreateInfo> shaders{};
        for (unsigned int i = 0; i < asset->shaderData.size(); i++) {
            VkShaderModule shaderModule;
            VkShaderModuleCreateInfo shaderModuleCreateInfo{};
            shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            shaderModuleCreateInfo.codeSize = asset->shaderData[i].size();
            shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t *>(asset->shaderData[i].data());
            if (vkCreateShaderModule(device.device, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) { throw std::runtime_error("failed to create shader module!"); }
            VkPipelineShaderStageCreateInfo shaderStageInfo{};
            shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStageInfo.module = shaderModule;
            shaderStageInfo.pName = "main";
            shaderStageInfo.stage = i ? VK_SHADER_STAGE_FRAGMENT_BIT : VK_SHADER_STAGE_VERTEX_BIT;
            shaders.push_back(shaderStageInfo);
        }
        //create graphics pipeline for this asset
        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{};
        vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        VkVertexInputBindingDescription bindingDescription = Vertex::getBindingDescription();
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = Vertex::getAttributeDescriptions();
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
        vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        VkPipelineInputAssemblyStateCreateInfo  inputAssemblyStateCreateInfo{};
        inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
        VkViewport viewport{};
        viewport.x = 0.f;
        viewport.y = 0.f;
        viewport.width = (float)swapchain.extent.width;
        viewport.height = (float)swapchain.extent.height;
        viewport.minDepth = 0.f;
        viewport.maxDepth = 1.f;
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapchain.extent;
        VkPipelineViewportStateCreateInfo viewportStateCreateInfo{};
        viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportStateCreateInfo.viewportCount = 1;
        viewportStateCreateInfo.pViewports = &viewport;
        viewportStateCreateInfo.scissorCount = 1;
        viewportStateCreateInfo.pScissors = &scissor;
        VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
        rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
        rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; //Controls fill mode (e.g. wireframe mode)
        rasterizationStateCreateInfo.lineWidth = 1.f;
        rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
        VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
        multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleStateCreateInfo.sampleShadingEnable = settings.msaaSamples == VK_SAMPLE_COUNT_1_BIT ? VK_FALSE : VK_TRUE;
        multisampleStateCreateInfo.rasterizationSamples = settings.msaaSamples;
        VkPipelineDepthStencilStateCreateInfo pipelineDepthStencilStateCreateInfo{};
        pipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
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
        VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo{};
        colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
        colorBlendStateCreateInfo.attachmentCount = 1;
        colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
        VkDynamicState dynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        VkPipelineDynamicStateCreateInfo pipelineDynamicStateCreateInfo{};
        pipelineDynamicStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        pipelineDynamicStateCreateInfo.dynamicStateCount = 2;
        pipelineDynamicStateCreateInfo.pDynamicStates = dynamicStates;
        VkGraphicsPipelineCreateInfo pipelineCreateInfo{};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
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
        pipelineCreateInfo.renderPass = asset->renderPass;
        pipelineCreateInfo.subpass = 0;
        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        if (vkCreateGraphicsPipelines(device.device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &asset->graphicsPipeline) != VK_SUCCESS) { throw std::runtime_error("failed to create graphics pipeline!"); }
        for (VkPipelineShaderStageCreateInfo shader : shaders) { vkDestroyShaderModule(device.device, shader.module, nullptr); }
        asset->deletionQueue.emplace_front([&](const Asset& thisAsset){ vkDestroyPipeline(device.device, thisAsset.graphicsPipeline, nullptr); });
        //create asset framebuffers
        asset->framebuffers.resize(swapchain.image_count);
        std::vector<VkImageView> framebufferAttachments{};
        for (unsigned int i = 0; i < asset->framebuffers.size(); i++) {
            if (settings.msaaSamples == VK_SAMPLE_COUNT_1_BIT) { framebufferAttachments = { swapchainImageViews[i], depthImage.view }; } else { framebufferAttachments = { colorImage.view, depthImage.view, swapchainImageViews[i] }; }
            VkFramebufferCreateInfo framebufferCreateInfo{};
            framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferCreateInfo.renderPass = asset->renderPass;
            framebufferCreateInfo.attachmentCount = framebufferAttachments.size();
            framebufferCreateInfo.pAttachments = framebufferAttachments.data();
            framebufferCreateInfo.width = swapchain.extent.width;
            framebufferCreateInfo.height = swapchain.extent.height;
            framebufferCreateInfo.layers = 1;
            if (vkCreateFramebuffer(device.device, &framebufferCreateInfo, nullptr, &asset->framebuffers[i]) != VK_SUCCESS) { throw std::runtime_error("failed to create framebuffers!"); }
        }
        asset->deletionQueue.emplace_front([&](const Asset& thisAsset){ for (VkFramebuffer framebuffer : thisAsset.framebuffers) { vkDestroyFramebuffer(device.device, framebuffer, nullptr); } });
        assets.push_back(asset);
    }

    void cleanUp() {
        for (Asset *asset : assets) { asset->destroy(); }
        for (std::function<void()>& function : recreationDeletionQueue) { function(); }
        recreationDeletionQueue.clear();
        for (std::function<void()>& function : oneTimeOptionalDeletionQueue) { function(); }
        oneTimeOptionalDeletionQueue.clear();
        for (std::function<void()>& function : engineDeletionQueue) { function(); }
        engineDeletionQueue.clear();
    }

    Settings settings{};
    GLFWwindow *window{};
    std::vector<Asset *> assets{};

private:
    void initializeVulkan(GLFWwindow *attachWindow) {
        framebufferResized = false;
        vkb::InstanceBuilder builder;
        vkb::detail::Result<vkb::Instance> inst_ret = builder.set_app_name(settings.applicationName.c_str()).set_app_version(settings.applicationVersion[0], settings.applicationVersion[1], settings.applicationVersion[2]).request_validation_layers().use_default_debug_messenger().build();
        if (!inst_ret) { throw std::runtime_error("Failed to create Vulkan instance. Error: " + inst_ret.error().message() + "\n"); }
        vkb::Instance vkb_inst = inst_ret.value();
        instance = inst_ret.value();
        engineDeletionQueue.emplace_front([&]{ vkb::destroy_instance(instance); });
        if (attachWindow == nullptr) { glfwInit(); }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(settings.resolution[0], settings.resolution[1], settings.applicationName.c_str(), settings.fullscreen ? glfwGetPrimaryMonitor() : nullptr, attachWindow);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        glfwSetWindowUserPointer(window, this);
        if (glfwCreateWindowSurface(instance.instance, window, nullptr, &surface) != VK_SUCCESS) { throw std::runtime_error("failed to create window surface!"); }
        engineDeletionQueue.emplace_front([&]{ vkDestroySurfaceKHR(instance.instance, surface, nullptr); });
        vkb::PhysicalDeviceSelector selector{ vkb_inst };
        VkPhysicalDeviceFeatures deviceFeatures{}; //Request device features here
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.sampleRateShading = VK_TRUE;
        vkb::detail::Result<vkb::PhysicalDevice> phys_ret = selector.set_surface(surface).set_minimum_version(settings.requiredVulkanVersion[0], settings.requiredVulkanVersion[1]).require_dedicated_transfer_queue().set_required_features(deviceFeatures).select();
        if (!phys_ret) { throw std::runtime_error("Failed to select Vulkan Physical Device. Error: " + phys_ret.error().message() + "\n"); }
        vkb::DeviceBuilder device_builder{ phys_ret.value() };
        vkb::detail::Result<vkb::Device> dev_ret = device_builder.build();
        if (!dev_ret) { throw std::runtime_error("Failed to create Vulkan device. Error: " + dev_ret.error().message() + "\n"); }
        device = dev_ret.value();
        engineDeletionQueue.emplace_front([&]{ vkb::destroy_device(device); });
        //Create commandPool
        VkCommandPoolCreateInfo commandPoolCreateInfo{};
        commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolCreateInfo.queueFamilyIndex = device.get_queue_index(vkb::QueueType::graphics).value();
        commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        if (vkCreateCommandPool(device.device, &commandPoolCreateInfo, nullptr, &commandPool) != VK_SUCCESS) { throw std::runtime_error("failed to create command pool!"); }
        engineDeletionQueue.emplace_front([&]{ vkDestroyCommandPool(device.device, commandPool, nullptr); });
        //delete staging buffer
        engineDeletionQueue.emplace_front([&]{ stagingBuffer.destroy(); });
        createSwapchain(true);
        //Create descriptor set layouts
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        VkDescriptorSetLayoutBinding albedoLayoutBinding{};
        albedoLayoutBinding.binding = 1;
        albedoLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        albedoLayoutBinding.descriptorCount = 1;
        albedoLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        std::vector<VkDescriptorSetLayoutBinding> bindings{uboLayoutBinding, albedoLayoutBinding};
        VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo{};
        descriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptorSetLayoutCreateInfo.bindingCount = bindings.size();
        descriptorSetLayoutCreateInfo.pBindings = bindings.data();
        if (vkCreateDescriptorSetLayout(device.device, &descriptorSetLayoutCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) { throw std::runtime_error("failed to create descriptor set layout!"); }
        engineDeletionQueue.emplace_front([&]{ vkDestroyDescriptorSetLayout(device.device, descriptorSetLayout, nullptr); });
        //Create pipelineLayout
        VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
        pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCreateInfo.setLayoutCount = 1;
        pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetLayout;
        if (vkCreatePipelineLayout(device.device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) { throw std::runtime_error("failed to create pipeline layout!"); }
        engineDeletionQueue.emplace_front([&]{ vkDestroyPipelineLayout(device.device, pipelineLayout, nullptr); });
    }

    void createSwapchain(bool fullRecreate = false, bool rebuildPipelines = false) {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        //Make sure no other GPU operations are on-going
        vkDeviceWaitIdle(device.device);
        //Clear recreationDeletionQueue
        for (std::function<void()>& function : recreationDeletionQueue) { function(); }
        recreationDeletionQueue.clear();
        if (fullRecreate) {
            for (std::function<void()>& function : oneTimeOptionalDeletionQueue) { function(); }
            oneTimeOptionalDeletionQueue.clear();
        }
        //Create swapchain
        vkb::SwapchainBuilder swapchainBuilder{ device };
        vkb::detail::Result<vkb::Swapchain> swap_ret = swapchainBuilder.set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR).build();
        if (!swap_ret) { throw std::runtime_error(swap_ret.error().message()); }
        swapchain = swap_ret.value();
        std::vector<VkImage> swapchainImages = swapchain.get_images().value();
        swapchainImageViews = swapchain.get_image_views().value();
        recreationDeletionQueue.emplace_front([&]{ vkb::destroy_swapchain(swapchain); });
        recreationDeletionQueue.emplace_front([&]{ swapchain.destroy_image_views(swapchainImageViews); });
        //Create output images
        colorImage.setEngineLink(renderEngineLink);
        colorImage.create(swapchain.image_format, VK_IMAGE_TILING_OPTIMAL, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, settings.msaaSamples, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, 1, swapchain.extent.width, swapchain.extent.height, false, false);
        recreationDeletionQueue.emplace_front([&]{ colorImage.destroy(); });
        depthImage.setEngineLink(renderEngineLink);
        depthImage.create(VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, settings.msaaSamples, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 1, swapchain.extent.width, swapchain.extent.height, true, false);
        depthImage.transition(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        recreationDeletionQueue.emplace_front([&]{ depthImage.destroy(); });
        //recreate framebuffers
        for (Asset *asset : assets) {
            for  (VkFramebuffer framebuffer : asset->framebuffers) { vkDestroyFramebuffer(device.device, framebuffer, nullptr); }
            asset->framebuffers.resize(swapchain.image_count);
            std::vector<VkImageView> framebufferAttachments{};
            for (unsigned int i = 0; i < asset->framebuffers.size(); i++) {
                if (settings.msaaSamples == VK_SAMPLE_COUNT_1_BIT) { framebufferAttachments = {swapchainImageViews[i], depthImage.view}; }
                else { framebufferAttachments = {colorImage.view, depthImage.view, swapchainImageViews[i]}; }
                VkFramebufferCreateInfo framebufferCreateInfo{};
                framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
                framebufferCreateInfo.renderPass = asset->renderPass;
                framebufferCreateInfo.attachmentCount = framebufferAttachments.size();
                framebufferCreateInfo.pAttachments = framebufferAttachments.data();
                framebufferCreateInfo.width = swapchain.extent.width;
                framebufferCreateInfo.height = swapchain.extent.height;
                framebufferCreateInfo.layers = 1;
                if (vkCreateFramebuffer(device.device, &framebufferCreateInfo, nullptr, &asset->framebuffers[i]) != VK_SUCCESS) { throw std::runtime_error("failed to create framebuffers!"); }
            }
        }
        //clear images marked as in flight
        imagesInFlight.clear();
        imagesInFlight.resize(swapchain.image_count, VK_NULL_HANDLE);
        //do the other stuff only if needed
        if (fullRecreate) {
            //Create commandBuffers
            commandBuffers.resize(swapchain.image_count);
            VkCommandBufferAllocateInfo commandBufferAllocateInfo{};
            commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            commandBufferAllocateInfo.commandPool = commandPool;
            commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            commandBufferAllocateInfo.commandBufferCount = commandBuffers.size();
            if (vkAllocateCommandBuffers(device.device, &commandBufferAllocateInfo, commandBuffers.data()) != VK_SUCCESS) { throw std::runtime_error("failed to allocate command buffers!"); }
            //Create sync objects
            imageAvailableSemaphores.resize(swapchain.image_count);
            renderFinishedSemaphores.resize(swapchain.image_count);
            inFlightFences.resize(swapchain.image_count);
            VkSemaphoreCreateInfo semaphoreCreateInfo{};
            semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
            VkFenceCreateInfo fenceCreateInfo{};
            fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
            for (unsigned int i = 0; i < swapchain.image_count; i++) {
                if (vkCreateSemaphore(device.device, &semaphoreCreateInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS) { throw std::runtime_error("failed to create semaphores!"); }
                if (vkCreateSemaphore(device.device, &semaphoreCreateInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS) { throw std::runtime_error("failed to create semaphores!"); }
                if (vkCreateFence(device.device, &fenceCreateInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) { throw std::runtime_error("failed to create fences!"); }
            }
            oneTimeOptionalDeletionQueue.emplace_front([&]{ for (VkSemaphore imageAvailableSemaphore : imageAvailableSemaphores) { vkDestroySemaphore(device.device, imageAvailableSemaphore, nullptr); } });
            oneTimeOptionalDeletionQueue.emplace_front([&]{ for (VkSemaphore renderFinishedSemaphore : renderFinishedSemaphores) { vkDestroySemaphore(device.device, renderFinishedSemaphore, nullptr); } });
            oneTimeOptionalDeletionQueue.emplace_front([&]{ for (VkFence inFlightFence : inFlightFences) { vkDestroyFence(device.device, inFlightFence, nullptr); } });
        }
    }

    static void framebufferResizeCallback(GLFWwindow *pWindow, int width, int height) {
        auto vulkanRenderEngine = (VulkanRenderEngine *)glfwGetWindowUserPointer(pWindow);
        vulkanRenderEngine->framebufferResized = true;
        vulkanRenderEngine->settings.resolution[0] = width;
        vulkanRenderEngine->settings.resolution[1] = height;
    }

    std::deque<std::function<void()>> recreationDeletionQueue{};
    std::deque<std::function<void()>> engineDeletionQueue{};
    std::deque<std::function<void()>> oneTimeOptionalDeletionQueue{};
    vkb::Device device{};
    RenderEngineLink renderEngineLink{};
    AllocatedBuffer stagingBuffer{};
    AllocatedImage colorImage{};
    AllocatedImage depthImage{};
    std::vector<VkCommandBuffer> commandBuffers{};
    VkSurfaceKHR surface{};
    std::vector<VkSemaphore> imageAvailableSemaphores{};
    std::vector<VkSemaphore> renderFinishedSemaphores{};
    std::vector<VkFence> inFlightFences{};
    std::vector<VkFence> imagesInFlight{};
    VkCommandPool commandPool{};
    vkb::Instance instance{};
    vkb::Swapchain swapchain{};
    std::vector<VkImageView> swapchainImageViews{};
    size_t currentFrame = 0;
    bool framebufferResized{false};
    VkDescriptorSetLayout descriptorSetLayout{};
    VkPipelineLayout pipelineLayout{};
};
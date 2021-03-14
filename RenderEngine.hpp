#pragma once

#include "Settings.hpp"
#include "Asset.hpp"
#include "AssetLinking.hpp"

class VulkanRenderEngine {
public:
    explicit VulkanRenderEngine(const Settings& startSettings) {
        settings = startSettings;
        createWindow();
        updateSettings(startSettings, false);
        createVulkanInstance();
        createWindowSurface();
        findBestSuitableDevice();
        createLogicalDevice();
        createMemoryAllocator();
        createSwapChain();
        createImageViews();
        createRenderPass();
        createDescriptorSetLayout();
        compileShaders("shaders");
        createGraphicsPipeline();
        createCommandPool();
        createResources();
        createFramebuffers();
    }

    RenderEngineLink createEngineLink() {
        RenderEngineLink engineLink{};
        engineLink.allocator = allocator;
        engineLink.settings = settings;
        engineLink.deletionQueue = deletionQueue;
        engineLink.logicalDevice = logicalDevice;
        engineLink.physicalDevice = physicalDevice;
        engineLink.swapChainImages = swapChainImages;
        engineLink.descriptorSetLayout = descriptorSetLayout;
        engineLink.descriptorPool = descriptorPool;
        engineLink.descriptorSets = descriptorSets;
        engineLink.uniformBuffers = uniformBuffers;
        engineLink.commandPool = commandPool;
        engineLink.singleTimeQueue = singleTimeQueue;
        return engineLink;
    }

    void recreateSwapChain() {
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        vkDeviceWaitIdle(logicalDevice);
        cleanupSwapChain();
        createSwapChain();
        createImageViews();
        createRenderPass();
        compileShaders("shaders");
        createDescriptorSetLayout();
        createGraphicsPipeline();
        createResources();
        createFramebuffers();
        createUniformBuffers();
        createDescriptorPool();
        createCommandBuffers();
    }

    void start() {
        createUniformBuffers();
        createDescriptorPool();
        createCommandBuffers();
        createSemaphores();
        createFences();
    }

    void updateSettings(const Settings& newSettings, bool updateAll) {
        if (!settings.fullscreen & newSettings.fullscreen) {glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, newSettings.resolution[0], newSettings.resolution[0], newSettings.refreshRate);}
        else if (settings.fullscreen & !newSettings.fullscreen) {glfwSetWindowMonitor(window, nullptr, 0, 0, 800, 600, newSettings.refreshRate);}
        else {glfwSetWindowSize(window, newSettings.resolution[0], newSettings.resolution[1]);}
        settings = newSettings;
        if (updateAll) {recreateSwapChain();}
    }

    int update() {
        if (glfwWindowShouldClose(window)) {
            return 1;}
        //Draw frame
        vkWaitForFences(logicalDevice, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(logicalDevice, swapChain, UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            return 0;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {throw std::runtime_error("failed to acquire swap chain image!");}
        if (imagesInFlight[imageIndex] != VK_NULL_HANDLE) {vkWaitForFences(logicalDevice, 1, &imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);}
        imagesInFlight[imageIndex] = inFlightFences[currentFrame];
        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        updateUniformBuffer(imageIndex);
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;
        vkResetFences(logicalDevice, 1, &inFlightFences[currentFrame]);
        if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {throw std::runtime_error("failed to submit draw command buffer!");}
        VkSwapchainKHR swapChains[] = {swapChain};
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        result = vkQueuePresentKHR(presentQueue, &presentInfo);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
            framebufferResized = false;
            recreateSwapChain();
        } else if (result != VK_SUCCESS) {throw std::runtime_error("failed to present swap chain image!");}
        vkQueueWaitIdle(presentQueue);
        currentFrame = (currentFrame + 1) % settings.MAX_FRAMES_IN_FLIGHT;
        frameNumber++;
        //Return success signal
        return 0;
    }

    ~VulkanRenderEngine() {
        cleanUp();
    }

    void cleanUp() {
        vkDeviceWaitIdle(logicalDevice);
        cleanupSwapChain();
        for (int i = 0; i < settings.MAX_FRAMES_IN_FLIGHT; i++) {
            vkDestroySemaphore(logicalDevice, renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(logicalDevice, imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(logicalDevice, inFlightFences[i], nullptr);
        }
        vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
        vkDestroyDevice(logicalDevice, nullptr);
        if (!settings.validationLayers.empty()) {DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);}
        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
        glfwSetWindowMonitor(window, nullptr, 0, 0, 800, 600, 0);
        glfwTerminate();
    }

    void cleanupSwapChain() {
        for (std::function<void()>& function : deletionQueue) {function();}
        deletionQueue.clear();
    }

    void createCommandBuffers() {
        commandBuffers.resize(swapChainFramebuffers.size());
        VkCommandBufferAllocateInfo commandBufferAllocInfo{};
        commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        commandBufferAllocInfo.commandPool = commandPool;
        commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        commandBufferAllocInfo.commandBufferCount = (uint32_t) commandBuffers.size();
        if (vkAllocateCommandBuffers(logicalDevice, &commandBufferAllocInfo, commandBuffers.data()) != VK_SUCCESS) { throw std::runtime_error("failed to allocate command buffers!"); }
        for (size_t i = 0; i < commandBuffers.size(); i++) {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("failed to begin recording command buffer!");
            }
            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
            clearValues[1].depthStencil = {1.0f, 0};
            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = renderPass;
            renderPassInfo.framebuffer = swapChainFramebuffers[i];
            renderPassInfo.renderArea.offset = {0, 0};
            renderPassInfo.renderArea.extent = swapChainExtent;
            renderPassInfo.clearValueCount = clearValues.size();
            renderPassInfo.pClearValues = clearValues.data();
            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
            if (!vertexBuffers.empty()) {
                vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers.data(), (VkDeviceSize *)vertexBuffers.size());
                vkCmdBindIndexBuffer(commandBuffers[i], indexBuffers[0], 0, VK_INDEX_TYPE_UINT32);
                vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,&descriptorSets[i], 0, nullptr);
                vkCmdDrawIndexed(commandBuffers[i], 10, 1, 0, 0, 0);
            }
            vkCmdEndRenderPass(commandBuffers[i]);
            if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {throw std::runtime_error("failed to record command buffer!");}
        }
    }

    std::array<std::vector<VkFence>, 2> createFences() {
        inFlightFences.resize(settings.MAX_FRAMES_IN_FLIGHT);
        imagesInFlight.resize(swapChainImages.size(), VK_NULL_HANDLE);
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        for (size_t i = 0; i < settings.MAX_FRAMES_IN_FLIGHT; i++) {if (vkCreateFence(logicalDevice, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {throw std::runtime_error("failed to create synchronization objects for a frame!");}}
        return {inFlightFences, imagesInFlight};
    }

    std::array<std::vector<VkSemaphore>, 2> createSemaphores() {
        imageAvailableSemaphores.resize(settings.MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(settings.MAX_FRAMES_IN_FLIGHT);
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        for (size_t i = 0; i < settings.MAX_FRAMES_IN_FLIGHT; i++) {if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS || vkCreateSemaphore(logicalDevice, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS) {throw std::runtime_error("failed to create synchronization objects for a frame!");}}
        return {imageAvailableSemaphores, renderFinishedSemaphores};
    }

    VkDescriptorPool createDescriptorPool() {
        std::array<VkDescriptorPoolSize, 2> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = swapChainImages.size();
        poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSizes[1].descriptorCount = swapChainImages.size();
        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = poolSizes.size();
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = swapChainImages.size();
        if (vkCreateDescriptorPool(logicalDevice, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {throw std::runtime_error("failed to create descriptor pool!");}
        deletionQueue.emplace_front([&]{vkDestroyDescriptorPool(logicalDevice, descriptorPool, nullptr);});
        return descriptorPool;
    }

    std::vector<VkBuffer> createUniformBuffers() {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);
        uniformBuffers.resize(swapChainImages.size());
        uniformBuffersMemory.resize(swapChainImages.size());
        for(size_t i =0; i < swapChainImages.size(); i++) {createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);}
        deletionQueue.emplace_front([&]{for (VkDeviceMemory memory : uniformBuffersMemory) {vkFreeMemory(logicalDevice, memory, nullptr);}});
        return uniformBuffers;
    }

    std::vector<VkFramebuffer> createFramebuffers() {
        swapChainFramebuffers.resize(swapChainImageViews.size());
        for (size_t i = 0; i < swapChainImageViews.size(); i++) {
            std::vector<VkImageView> attachments{swapChainImageViews[i], depthImageView};
            if (settings.msaaSamples != VK_SAMPLE_COUNT_1_BIT) {attachments = {colorImageView, depthImageView, swapChainImageViews[i]};}
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = renderPass;
            framebufferInfo.attachmentCount = attachments.size();
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtent.width;
            framebufferInfo.height = swapChainExtent.height;
            framebufferInfo.layers = 1;
            if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {throw std::runtime_error("failed to create framebuffer!");}
        }
        deletionQueue.emplace_front([&]{for (VkFramebuffer framebuffer : swapChainFramebuffers)vkDestroyFramebuffer(logicalDevice, framebuffer, nullptr);});
        return swapChainFramebuffers;
    }

    std::array<VkImage, 2> createResources() {
        VkFormat colorFormat = swapChainImageFormat;
        createImage(swapChainExtent.width, swapChainExtent.height, 1, settings.msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, colorImage, colorImageMemory);
        colorImageView = createImageView(colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
        deletionQueue.emplace_front([&]{vkDestroyImageView(logicalDevice, colorImageView, nullptr);});
        deletionQueue.emplace_front([&]{vkFreeMemory(logicalDevice, colorImageMemory, nullptr);});
        createImage(swapChainExtent.width, swapChainExtent.height, 1, settings.msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
        depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1);
        deletionQueue.emplace_front([&]{vkDestroyImageView(logicalDevice, depthImageView, nullptr);});
        deletionQueue.emplace_front([&]{vkFreeMemory(logicalDevice, depthImageMemory, nullptr);});
        return {colorImage, depthImage};
    }

    VkCommandPool createCommandPool() {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = familyIndices.graphicsFamily.value();
        poolInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
        if (vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {throw std::runtime_error("failed to create command pool!");}
        return commandPool;
    }

    VkPipeline createGraphicsPipeline() {
        VkShaderModule vertShaderModule = readShader("shaders/vertexShader.spv");
        VkShaderModule fragShaderModule = readShader("shaders/fragmentShader.spv");
        VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
        vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertShaderStageInfo.module = vertShaderModule;
        vertShaderStageInfo.pName = "main";
        VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
        fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragShaderStageInfo.module = fragShaderModule;
        fragShaderStageInfo.pName = "main";
        VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};
        VkVertexInputBindingDescription bindingDescriptions = Vertex::getBindingDescription();
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = Vertex::getAttributeDescriptions();
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescriptions;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssembly.primitiveRestartEnable = VK_FALSE;
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = (float) swapChainExtent.width;
        viewport.height = (float) swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapChainExtent;
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = &viewport;
        viewportState.scissorCount = 1;
        viewportState.pScissors = &scissor;
        VkPipelineRasterizationStateCreateInfo rasterizer{};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;
        rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizer.lineWidth = 1.0f;
        rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        VkPipelineMultisampleStateCreateInfo multisampling{};
        multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        if (settings.msaaSamples == 1) {multisampling.sampleShadingEnable = VK_FALSE;}
        else {
            multisampling.sampleShadingEnable = VK_TRUE;
            multisampling.minSampleShading = .2f;}
        multisampling.rasterizationSamples = settings.msaaSamples;
        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;
        VkPipelineColorBlendStateCreateInfo colorBlending{};
        colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlending.logicOpEnable = VK_FALSE;
        colorBlending.logicOp = VK_LOGIC_OP_COPY;
        colorBlending.attachmentCount = 1;
        colorBlending.pAttachments = &colorBlendAttachment;
        colorBlending.blendConstants[0] = 0.0f;
        colorBlending.blendConstants[1] = 0.0f;
        colorBlending.blendConstants[2] = 0.0f;
        colorBlending.blendConstants[3] = 0.0f;
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {throw std::runtime_error("failed to create pipeline layout!");}
        deletionQueue.emplace_front([&]{vkDestroyPipelineLayout(logicalDevice, pipelineLayout, nullptr);});
        VkPipelineDepthStencilStateCreateInfo  depthStencil{};
        depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthStencil.depthTestEnable = VK_TRUE;
        depthStencil.depthWriteEnable = VK_TRUE;
        depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
        depthStencil.depthBoundsTestEnable = VK_FALSE;
        depthStencil.stencilTestEnable = VK_FALSE;
        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDepthStencilState = &depthStencil;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {throw std::runtime_error("failed to create graphics pipeline!");}
        deletionQueue.emplace_front([&]{vkDestroyPipeline(logicalDevice, graphicsPipeline, nullptr);});
        vkDestroyShaderModule(logicalDevice, fragShaderModule, nullptr);
        vkDestroyShaderModule(logicalDevice, vertShaderModule, nullptr);
        return graphicsPipeline;
    }

    VkDescriptorSetLayout createDescriptorSetLayout() {
        VkDescriptorSetLayoutBinding uboLayoutBinding{};
        uboLayoutBinding.binding = 0;
        uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        uboLayoutBinding.descriptorCount = 1;
        uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        VkDescriptorSetLayoutBinding samplerLayoutBinding{};
        samplerLayoutBinding.binding = 1;
        samplerLayoutBinding.descriptorCount = 1;
        samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        samplerLayoutBinding.pImmutableSamplers = nullptr;
        samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();
        if (vkCreateDescriptorSetLayout(logicalDevice, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {throw std::runtime_error("failed to create descriptor set layout!");}
        deletionQueue.emplace_front([&]{vkDestroyDescriptorSetLayout(logicalDevice, descriptorSetLayout, nullptr);});
        return descriptorSetLayout;
    };

    VkRenderPass createRenderPass() {
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = swapChainImageFormat;
        colorAttachment.samples = settings.msaaSamples;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        if (settings.msaaSamples == VK_SAMPLE_COUNT_1_BIT) {colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;}
        else {colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;}
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        for (VkFormat format : {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}) {
            VkFormatProperties formatProperties{};
            vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
            if ((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {depthFormat = format;}
        }
        if (!depthFormat) {throw std::runtime_error("unable to find usable format!");}
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = depthFormat;
        depthAttachment.samples = settings.msaaSamples;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        VkAttachmentDescription colorAttachmentResolve{};
        colorAttachmentResolve.format = swapChainImageFormat;
        colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        VkAttachmentReference colorAttachmentResolveRef{};
        colorAttachmentResolveRef.attachment = 2;
        colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;
        if (settings.msaaSamples == VK_SAMPLE_COUNT_1_BIT) {subpass.pResolveAttachments = nullptr;}
        else {subpass.pResolveAttachments = &colorAttachmentResolveRef;}
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        if (settings.msaaSamples == VK_SAMPLE_COUNT_1_BIT) {
            renderPassInfo.attachmentCount = 2;
            renderPassInfo.pAttachments = std::array<VkAttachmentDescription, 2> {colorAttachment, depthAttachment}.data();
        }
        else {
            renderPassInfo.attachmentCount = 3;
            renderPassInfo.pAttachments = std::array<VkAttachmentDescription, 3> {colorAttachment, depthAttachment, colorAttachmentResolve}.data();
        }
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;
        if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {throw std::runtime_error("failed to create render pass!");}
        deletionQueue.emplace_front([&]{vkDestroyRenderPass(logicalDevice, renderPass, nullptr);});
        return renderPass;
    }

    std::vector<VkImageView> createImageViews() {
        swapChainImageViews.resize(swapChainImages.size());
        for (size_t i = 0; i < swapChainImages.size(); i++) {swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);}
        deletionQueue.emplace_front([&]{for (VkImageView imageView : swapChainImageViews) {vkDestroyImageView(logicalDevice, imageView, nullptr);}});
        return swapChainImageViews;
    }

    VkSwapchainKHR createSwapChain() {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &swapChainSupport.capabilities);
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
        if (formatCount != 0) {
            swapChainSupport.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, swapChainSupport.formats.data());
        }
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
        if(presentModeCount != 0) {
            swapChainSupport.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, swapChainSupport.presentModes.data());
        }
        //Choose best format
        VkSurfaceFormatKHR surfaceFormat;
        bool chosen = false;
        for (const auto& availableFormat : swapChainSupport.formats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                surfaceFormat = availableFormat;
                chosen = true;
                break;
            }
        }
        if (!chosen) {surfaceFormat = swapChainSupport.formats[0];}
        //Choose best present mode
        VkPresentModeKHR presentMode;
        chosen = false;
        for (const auto& availablePresentMode : swapChainSupport.presentModes) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                presentMode = availablePresentMode;
                chosen = true;
                break;
            }
        }
        if (!chosen) {presentMode = VK_PRESENT_MODE_FIFO_KHR;}
        //Choose swap chain extent
        VkExtent2D extent;
        if (swapChainSupport.capabilities.currentExtent.width != UINT32_MAX) {extent = swapChainSupport.capabilities.currentExtent;}
        else {
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            extent = {
                    static_cast<uint32_t>(width),
                    static_cast<uint32_t>(height)
            };
            extent.width = (std::max)(swapChainSupport.capabilities.minImageExtent.width, (std::min)(swapChainSupport.capabilities.maxImageExtent.width, extent.width));
            extent.height = (std::max)(swapChainSupport.capabilities.minImageExtent.height, (std::min)(swapChainSupport.capabilities.maxImageExtent.height, extent.height));
        }
        //Create swap chain
        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {imageCount = swapChainSupport.capabilities.maxImageCount;}
        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        uint32_t queueFamilyIndices[] = {familyIndices.graphicsFamily.value(), familyIndices.presentFamily.value()};
        if (familyIndices.graphicsFamily != familyIndices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        } else {createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;}
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;
        if (vkCreateSwapchainKHR(logicalDevice, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {throw std::runtime_error("failed to create swap chain!");}
        deletionQueue.emplace_front([&]{vkDestroySwapchainKHR(logicalDevice, swapChain, nullptr);});
        vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, swapChainImages.data());
        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
        return swapChain;
    }

    VmaAllocator createMemoryAllocator() {
        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.physicalDevice = physicalDevice;
        allocatorInfo.device = logicalDevice;
        allocatorInfo.instance = instance;
        vmaCreateAllocator(&allocatorInfo, &allocator);
        return allocator;
    }

    VkDevice createLogicalDevice() {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) { familyIndices.graphicsFamily = i;}
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
            if (presentSupport) { familyIndices.presentFamily = i;}
            if (familyIndices.presentFamily.has_value() && familyIndices.graphicsFamily.has_value()) {break;}
            i++;
        }
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {familyIndices.graphicsFamily.value(), familyIndices.presentFamily.value()};
        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }
        physicalDeviceFeatures.samplerAnisotropy = (settings.anisotropicFilterLevel > 1) ? VK_TRUE: VK_FALSE;
        physicalDeviceFeatures.sampleRateShading = VK_TRUE;
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &physicalDeviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(settings.requestedExtensions.size());
        createInfo.ppEnabledExtensionNames = settings.requestedExtensions.data();
        if (!settings.validationLayers.empty()) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(settings.validationLayers.size());
            createInfo.ppEnabledLayerNames = settings.validationLayers.data();
        } else {createInfo.enabledLayerCount = 0;}
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS) {throw std::runtime_error("failed to create logical device!");}
        vkGetDeviceQueue(logicalDevice, familyIndices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(logicalDevice, familyIndices.presentFamily.value(), 0, &presentQueue);
        return logicalDevice;
    }

    VkPhysicalDevice findBestSuitableDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
        long unsigned int highestMemorySize = 0;
        long unsigned int deviceMemorySize;
        for (auto & singleDevice : devices) {
            uint32_t extensionCount;
            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(singleDevice, nullptr, &extensionCount, availableExtensions.data());
            std::set<std::string> requiredExtensions(settings.requestedExtensions.begin(), settings.requestedExtensions.end());
            for (const auto& extension : availableExtensions) {
                requiredExtensions.erase(extension.extensionName);
            }
            if (!requiredExtensions.empty()) {continue;}
            vkGetPhysicalDeviceProperties(singleDevice, &physicalDeviceProperties);
            if (physicalDeviceProperties.deviceType == 2) {
                vkGetPhysicalDeviceMemoryProperties(singleDevice, &physicalDeviceMemoryProperties);
                deviceMemorySize = physicalDeviceMemoryProperties.memoryHeaps[0].size;
                if (deviceMemorySize > highestMemorySize) {
                    highestMemorySize = deviceMemorySize;
                    physicalDevice = singleDevice;
                }
            }
        }
        if (physicalDevice == VK_NULL_HANDLE) {throw std::runtime_error("None of the GPUs in your system support Vulkan. If you know this to be false try updating your drivers.");}
        return physicalDevice;
    }

    VkSurfaceKHR createWindowSurface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {throw std::runtime_error("failed to create window surface!");}
        return surface;
    }

    VkInstance createVulkanInstance() {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = settings.applicationName.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(settings.applicationVersion[0], settings.applicationVersion[1], settings.applicationVersion[2]);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
        appInfo.apiVersion = VK_API_VERSION_1_2;
        uint32_t glfwExtensionCount = 0;
        const char* *glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
        if (!settings.validationLayers.empty()) {extensions.push_back("VK_EXT_debug_utils");}
        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = extensions.size();
        createInfo.ppEnabledExtensionNames = extensions.data();
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (!settings.validationLayers.empty()) {
            //Check for validation layer support
            uint32_t layerCount;
            vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
            std::vector<VkLayerProperties> availableLayers(layerCount);
            vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
            for (const char *layerName : settings.validationLayers) {
                bool layerFound = false;
                for (const auto& layerProperties : availableLayers) {
                    if (strcmp(layerName, layerProperties.layerName) == 0) {
                        layerFound = true;
                        break;
                    }
                }
                if (!layerFound) {throw std::runtime_error("validation layers requested, but not available!");}
            }
            //Setup debugger
            debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugCreateInfo.pfnUserCallback = debugCallback;
            createInfo.enabledLayerCount = static_cast<uint32_t>(settings.validationLayers.size());
            createInfo.ppEnabledLayerNames = settings.validationLayers.data();
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
        } else {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }
        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {throw std::runtime_error("failed to create Vulkan instance!");}
        if (!settings.validationLayers.empty()) {if (CreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS) {throw std::runtime_error("failed to setup debug messenger!");}}
        return instance;
    }

    GLFWwindow *createWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(settings.resolution[0], settings.resolution[1], settings.applicationName.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        glfwSetWindowUserPointer(window, this);
        return window;
    }

    VkShaderModule readShader(const char *filename) const {
        std::ifstream file(settings.absolutePath + filename, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {throw std::runtime_error("failed to open file: " + std::string(filename) + "\n as file: " + settings.absolutePath + filename);}
        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = buffer.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {throw std::runtime_error("failed to create shader module!");}
        return shaderModule;
    }

    void compileShaders(const char *filename) const {
        for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(settings.absolutePath + filename)) {
            std::string path = entry.path().string();
            if (entry.path().extension() == ".frag" || entry.path().extension() == ".vert") {
                system((GLSLC + path + " -o " + path.substr(0, path.find_last_of('.')) + ".spv").c_str());}
        }
        for (const std::filesystem::directory_entry& file : std::filesystem::directory_iterator(settings.absolutePath + filename)) {if (file.path().extension() == ".spv") {return;}}
        throw std::runtime_error("no precompiled shaders exist and compiling failed as glslc was not found!");
    }

    void updateUniformBuffer(uint32_t currentImage) {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
//        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        UniformBufferObject ubo{};
        ubo.model = glm::rotate(glm::mat4(1.0f), glm::radians(0.f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float) swapChainExtent.height, 0.1f, 10.0f);
        ubo.proj[1][1] *= -1;
        void *data;
        vkMapMemory(logicalDevice, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
        memcpy(data, &ubo, sizeof(ubo));
        vkUnmapMemory(logicalDevice, uniformBuffersMemory[currentImage]);
    }

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands(1)[0];
        VkBufferCopy copyRegion{};
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
        endSingleTimeCommands(std::vector<VkCommandBuffer>{commandBuffer});
    }

    [[nodiscard]] std::vector<VkCommandBuffer> beginSingleTimeCommands(size_t count) const {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = commandPool;
        allocInfo.commandBufferCount = count;
        std::vector<VkCommandBuffer> singleTimeCommandBuffers{count};
        vkAllocateCommandBuffers(logicalDevice, &allocInfo, singleTimeCommandBuffers.data());
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        for (VkCommandBuffer singleTimeCommandBuffer : singleTimeCommandBuffers) {vkBeginCommandBuffer(singleTimeCommandBuffer, &beginInfo);}
        return singleTimeCommandBuffers;
    }

    void endSingleTimeCommands(std::vector<VkCommandBuffer> singleTimeCommandBuffers) const {
        for (VkCommandBuffer singleTimeCommandBuffer : singleTimeCommandBuffers) {vkEndCommandBuffer(singleTimeCommandBuffer);}
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = singleTimeCommandBuffers.size();
        submitInfo.pCommandBuffers = singleTimeCommandBuffers.data();
        vkQueueSubmit(singleTimeQueue, singleTimeCommandBuffers.size(), &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(singleTimeQueue);
        vkFreeCommandBuffers(logicalDevice, commandPool, singleTimeCommandBuffers.size(), singleTimeCommandBuffers.data());
    }

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        if (vkCreateBuffer(logicalDevice, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {throw std::runtime_error("failed to create buffer!");}
        deletionQueue.emplace_front([&]{vkDestroyBuffer(logicalDevice, buffer, nullptr);});
        uint32_t memoryIndex;
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(logicalDevice, buffer, &memRequirements);
        for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++) {
            if ((memRequirements.memoryTypeBits & (1 << i)) && (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                memoryIndex = i;
                break;
            }
        }
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = memoryIndex;
        if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {throw std::runtime_error("failed to allocate buffer memory!");}
        vkBindBufferMemory(logicalDevice, buffer, bufferMemory, 0);
    }

    VkImage createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = numSamples;
        if (vkCreateImage(logicalDevice, &imageInfo, nullptr, &image) != VK_SUCCESS) {throw std::runtime_error("failed to create image!");}
        deletionQueue.emplace_front([&]{vkDestroyImage(logicalDevice, image, nullptr);});
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(logicalDevice, image, &memRequirements);
        uint32_t memoryIndex;
        for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++) {
            if ((memRequirements.memoryTypeBits & (1 << i)) &&
                (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                memoryIndex = i;
                break;
            }
        }
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = memoryIndex;
        if (vkAllocateMemory(logicalDevice, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {throw std::runtime_error("failed to allocate image memory!");}
        vkBindImageMemory(logicalDevice, image, imageMemory, 0);
        return image;
    }

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels) {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        VkImageView imageView{};
        if (vkCreateImageView(logicalDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {throw std::runtime_error("failed to create image view!");}
        return imageView;
    }

    static void framebufferResizeCallback(GLFWwindow *window, int width, int height) {
        reinterpret_cast<VulkanRenderEngine*>(glfwGetWindowUserPointer(window))->framebufferResized = true;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {return func(instance, pCreateInfo, pAllocator, pDebugMessenger);}
        else {return VK_ERROR_EXTENSION_NOT_PRESENT;}
    }

    static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {func(instance, debugMessenger, pAllocator);}
    }

    GLFWwindow *window{};
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    Settings settings{};

private:
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities{};
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    VmaAllocator allocator{};
    uint32_t frameNumber = 0;
    std::deque<std::function<void()>> deletionQueue{};
    VkDevice logicalDevice{};
    VkInstance instance{};
    VkDebugUtilsMessengerEXT debugMessenger{};
    bool framebufferResized = false;
    VkSurfaceKHR surface{};
    VkPhysicalDeviceProperties physicalDeviceProperties{};
    VkPhysicalDeviceFeatures physicalDeviceFeatures{};
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties{};
    QueueFamilyIndices familyIndices{};
    VkQueue graphicsQueue{};
    VkQueue singleTimeQueue{};
    VkQueue presentQueue{};
    SwapChainSupportDetails swapChainSupport;
    VkSwapchainKHR swapChain{};
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat{};
    VkExtent2D swapChainExtent{};
    std::vector<VkImageView> swapChainImageViews;
    VkFormat depthFormat{};
    VkRenderPass renderPass{};
    VkDescriptorSetLayout descriptorSetLayout{};
    VkPipelineLayout pipelineLayout{};
    VkPipeline graphicsPipeline{};
    VkCommandPool commandPool{};
    VkImage colorImage{};
    VkDeviceMemory colorImageMemory{};
    VkImageView colorImageView{};
    VkImage depthImage{};
    VkDeviceMemory depthImageMemory{};
    VkImageView depthImageView{};
    std::vector<VkFramebuffer> swapChainFramebuffers{};
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    VkDescriptorPool descriptorPool{};
    std::vector<VkDescriptorSet> descriptorSets;
    VkCommandBuffer mainCommandBuffer{};
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;
    std::vector<VkBuffer> vertexBuffers{};
    std::vector<VkBuffer> indexBuffers{};
    std::vector<Asset> assets{};
};

class OpenGLRenderEngine {
public:
    Settings settings{};
    GLFWwindow *window{};
    GLuint vertexBuffer{};
    GLuint programID{};

    explicit OpenGLRenderEngine(const Settings& startSettings) {
        settings = startSettings;
        if(!glfwInit()) {throw std::runtime_error("failed to initialize GLFW");}
        glfwWindowHint(GLFW_SAMPLES, settings.msaaSamples);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        window = glfwCreateWindow(settings.resolution[0], settings.resolution[1], settings.applicationName.c_str(), nullptr, nullptr);
        if (window == nullptr) {throw std::runtime_error("failed to open GLFW window!");}
        glfwMakeContextCurrent(window);
        glewExperimental = true;
        if (glewInit() != GLEW_OK) {throw std::runtime_error("failed to initialize GLEW!");}
        GLuint VertexArrayID;
        glGenVertexArrays(1, &VertexArrayID);
        glBindVertexArray(VertexArrayID);
        //Create vertex buffer
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
        programID = loadShaders({"shaders/vertexShader.glsl", "shaders/fragmentShader.glsl"});
    }

    [[nodiscard]] int update() const {
        if (glfwWindowShouldClose(window)) {
            return 1;
        }
        glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(programID);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glDisableVertexAttribArray(0);
        glfwSwapBuffers(window);
        return 0;
    }

    ~OpenGLRenderEngine() {
        cleanUp();
    }

private:
    constexpr static const GLfloat g_vertex_buffer_data[] = {-1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,  1.0f, 0.0f};

    static void cleanUp() {
        glFinish();
        glfwTerminate();
    }

    [[nodiscard]] GLuint loadShaders(const std::array<std::string, 2>& paths) const {
        GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
        std::array<GLuint, 2> shaderIDs = {vertexShaderID, fragmentShaderID};
        GLint Result = GL_FALSE;
        int InfoLogLength;
        for (int i = 0; i < paths.size(); i++) {
            std::ifstream file(settings.absolutePath + paths[i], std::ios::in);
            if (!file.is_open()) {throw std::runtime_error("failed to load shader: " + settings.absolutePath + paths[i]);}
            std::stringstream stringStream;
            stringStream << file.rdbuf();
            std::string shaderCode = stringStream.str();
            file.close();
            char const * sourcePointer = shaderCode.c_str();
            glShaderSource(shaderIDs[i], 1, &sourcePointer, nullptr);
            glCompileShader(shaderIDs[i]);
            glGetShaderiv(shaderIDs[i], GL_COMPILE_STATUS, &Result);
            glGetShaderiv(shaderIDs[i], GL_INFO_LOG_LENGTH, &InfoLogLength);
            if (InfoLogLength > 0){throw std::runtime_error("failed to compile shaders!");}
        }
        GLuint ProgramID = glCreateProgram();
        glAttachShader(ProgramID, vertexShaderID);
        glAttachShader(ProgramID, fragmentShaderID);
        glLinkProgram(ProgramID);
        glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
        glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
        if (InfoLogLength > 0){
            std::vector<char> ProgramErrorMessage(InfoLogLength+1);
            glGetProgramInfoLog(ProgramID, InfoLogLength, nullptr, &ProgramErrorMessage[0]);
            printf("%s\n", &ProgramErrorMessage[0]);
        }
        glDetachShader(ProgramID, vertexShaderID);
        glDetachShader(ProgramID, fragmentShaderID);
        glDeleteShader(vertexShaderID);
        glDeleteShader(fragmentShaderID);
        return ProgramID;
    }
};

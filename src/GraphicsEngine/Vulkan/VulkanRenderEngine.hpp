#pragma once

#include <deque>
#include <functional>
#include <vector>

#include <VkBootstrap.h>

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "Asset.hpp"
#include "BufferManager.hpp"
#include "Camera.hpp"
#include "CommandBufferManager.hpp"
#include "DescriptorSetManager.hpp"
#include "GPUData.hpp"
#include "ImageManager.hpp"
#include "RenderPassManager.hpp"
#include "Vertex.hpp"
#include "VulkanGraphicsEngineLink.hpp"

class VulkanRenderEngine {
private:
    vkb::Instance instance{};
    std::deque<std::function<void()>> engineDeletionQueue{};
    VulkanGraphicsEngineLink renderEngineLink{};
    BufferManager stagingBuffer{};
    VmaAllocator allocator{};
    bool framebufferResized{false};
    VkSurfaceKHR surface{};
    std::deque<std::function<void()>> recreationDeletionQueue{};
    std::deque<std::function<void()>> oneTimeOptionalDeletionQueue{};
    DescriptorSetManager descriptorSetManager{};
    std::vector<VkImageView> swapchainImageViews{};
    void *vulkan_library{};

protected:
    virtual bool update() { return false; }

    explicit VulkanRenderEngine(Settings &initialSettings = *new Settings{}, GLFWwindow *attachWindow = nullptr) {
        #if defined _WIN32
        vulkan_library = LoadLibrary("vulkan-1.dll");
        #else
        vulkan_library = dlopen("libvulkan.so.1", RTLD_NOW);
        #endif
        if( vulkan_library == nullptr ) { throw std::runtime_error("Could not connect with a Vulkan Runtime library."); }
        settings = &initialSettings;
        renderEngineLink.device = &device;
        renderEngineLink.swapchain = &swapchain;
        renderEngineLink.settings = settings;
        renderEngineLink.commandPool = &commandBufferManager.commandPool;
        renderEngineLink.allocator = &allocator;
        framebufferResized = false;
        vkb::detail::Result<vkb::SystemInfo> systemInfo = vkb::SystemInfo::get_system_info();
        //build instance
        vkb::InstanceBuilder builder;
        builder.set_app_name(settings->applicationName.c_str()).set_app_version(settings->applicationVersion[0], settings->applicationVersion[1], settings->applicationVersion[2]).require_api_version(settings->requiredVulkanVersion[0], settings->requiredVulkanVersion[1], settings->requiredVulkanVersion[2]);
        if (systemInfo->validation_layers_available) { builder.request_validation_layers(); }
        if (systemInfo->debug_utils_available) { builder.use_default_debug_messenger(); }
        vkb::detail::Result <vkb::Instance> inst_ret = builder.build();
        if (!inst_ret) { throw std::runtime_error("Failed to create Vulkan instance. Error: " + inst_ret.error().message() + "\n"); }
        instance = inst_ret.value();
        //build window
        engineDeletionQueue.emplace_front([&] { vkb::destroy_instance(instance); });
        if (attachWindow == nullptr) {
            glfwInit();
            settings->monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode *mode = glfwGetVideoMode(settings->monitor);
            settings->defaultMonitorResolution = {mode->width, mode->height};
        }
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(settings->resolution[0], settings->resolution[1], settings->applicationName.c_str(), settings->fullscreen ? settings->monitor : nullptr, attachWindow);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        glfwSetWindowUserPointer(window, this);
        if (glfwCreateWindowSurface(instance.instance, window, nullptr, &surface) != VK_SUCCESS) { throw std::runtime_error("failed to create window surface!"); }
        engineDeletionQueue.emplace_front([&] { vkDestroySurfaceKHR(instance.instance, surface, nullptr); });
        //select physical device
        vkb::PhysicalDeviceSelector selector{instance};
        std::vector<const char *> extensionNames{systemInfo->available_extensions.size()};
        for (int i = 0; i < systemInfo->available_extensions.size(); ++i) { extensionNames[i] = systemInfo->available_extensions[i].extensionName; } //Enable all available extensions
        extensionNames.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
        extensionNames.push_back(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME); //^
        extensionNames.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME); //^^
        extensionNames.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME); //^^^
        extensionNames.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
        extensionNames.push_back(VK_KHR_SPIRV_1_4_EXTENSION_NAME); //^
        extensionNames.push_back(VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME); //^
        for (const char *name : extensionNames) { std::cout << "Using extension: " << name << std::endl; }
        VkPhysicalDeviceFeatures deviceFeatures{}; //Request device features here
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.sampleRateShading = VK_TRUE;
        vkb::detail::Result <vkb::PhysicalDevice> phys_ret = selector.set_surface(surface).require_dedicated_transfer_queue().add_desired_extensions(extensionNames).set_required_features(deviceFeatures).select();
        if (!phys_ret) { throw std::runtime_error("Failed to select Vulkan Physical Device. Error: " + phys_ret.error().message() + "\n"); }
        //create logical device
        vkb::DeviceBuilder device_builder{phys_ret.value()};
        vkb::detail::Result <vkb::Device> dev_ret = device_builder.build();
        if (!dev_ret) { throw std::runtime_error("Failed to create Vulkan device. Error: " + dev_ret.error().message() + "\n"); }
        device = dev_ret.value();
        engineDeletionQueue.emplace_front([&] { vkb::destroy_device(device); });
        //get queues
        graphicsQueue = device.get_queue(vkb::QueueType::graphics).value();
        presentQueue = device.get_queue(vkb::QueueType::present).value();
        //create vma allocator
        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.physicalDevice = device.physical_device.physical_device;
        allocatorInfo.device = device.device;
        allocatorInfo.instance = instance.instance;
        vmaCreateAllocator(&allocatorInfo, &allocator);
        engineDeletionQueue.emplace_front([&] { vmaDestroyAllocator(allocator); });
        //Create commandPool
        commandBufferManager.setup(device, vkb::QueueType::graphics);
        engineDeletionQueue.emplace_front([&] { commandBufferManager.destroy(); });
        //delete staging buffer
        stagingBuffer.setEngineLink(&renderEngineLink);
        engineDeletionQueue.emplace_front([&] { stagingBuffer.destroy(); });
        createSwapchain(true);
    }

    void createSwapchain(bool fullRecreate = false) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        //Make sure no other GPU operations are on-going
        vkDeviceWaitIdle(device.device);
        //Clear recreationDeletionQueue
        for (std::function<void()>& function : recreationDeletionQueue) { function(); }
        recreationDeletionQueue.clear();
        //Create swapchain
        vkb::SwapchainBuilder swapchainBuilder{ device };
        vkb::detail::Result<vkb::Swapchain> swap_ret = swapchainBuilder.set_desired_present_mode(VK_PRESENT_MODE_IMMEDIATE_KHR).build();
        if (!swap_ret) { throw std::runtime_error(swap_ret.error().message()); }
        swapchain = swap_ret.value();
        swapchainImageViews = swapchain.get_image_views().value();
        renderEngineLink.swapchainImageViews = &swapchainImageViews;
        recreationDeletionQueue.emplace_front([&]{ vkb::destroy_swapchain(swapchain); });
        recreationDeletionQueue.emplace_front([&]{ swapchain.destroy_image_views(swapchainImageViews); });
        //clear images marked as in flight
        imagesInFlight.clear();
        imagesInFlight.resize(swapchain.image_count, VK_NULL_HANDLE);
        //do the other stuff only if needed
        if (fullRecreate) {
            for (std::function<void()>& function : oneTimeOptionalDeletionQueue) { function(); }
            oneTimeOptionalDeletionQueue.clear();
            //Create descriptorSetManager
            descriptorSetManager.setup({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER}, {VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT}, swapchain.image_count, device.device);
            oneTimeOptionalDeletionQueue.emplace_front([&]{ descriptorSetManager.destroy(); });
            //Create pipelineLayout
            VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
            pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pipelineLayoutCreateInfo.setLayoutCount = 1;
            pipelineLayoutCreateInfo.pSetLayouts = &descriptorSetManager.descriptorSetLayout;
            if (vkCreatePipelineLayout(device.device, &pipelineLayoutCreateInfo, nullptr, &pipelineLayout) != VK_SUCCESS) { throw std::runtime_error("failed to create pipeline layout!"); }
            oneTimeOptionalDeletionQueue.emplace_front([&]{ vkDestroyPipelineLayout(device.device, pipelineLayout, nullptr);});
            //Create commandBuffers
            commandBufferManager.createCommandBuffers((int)swapchain.image_count);
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
            //Create render pass
            renderPassManager.setup(&renderEngineLink);
            oneTimeOptionalDeletionQueue.emplace_front([&]{ renderPassManager.destroy(); });
            for (Asset *asset : assets) {
                uploadAsset(asset, false);
            }
        }
        //recreate framebuffers
        renderPassManager.recreateFramebuffers();
        //update camera
        camera.resolution = settings->resolution;
        camera.fov = settings->fov;
        camera.renderDistance = settings->renderDistance;
    }

    static void framebufferResizeCallback(GLFWwindow *pWindow, int width, int height) {
        auto vulkanRenderEngine = (VulkanRenderEngine *)glfwGetWindowUserPointer(pWindow);
        vulkanRenderEngine->framebufferResized = true;
        vulkanRenderEngine->settings->resolution[0] = width;
        vulkanRenderEngine->settings->resolution[1] = height;
    }

    vkb::Device device{};
    std::vector<VkFence> inFlightFences{};
    vkb::Swapchain swapchain{};
    std::vector<VkFence> imagesInFlight{};
    std::vector<VkSemaphore> imageAvailableSemaphores{};
    std::vector<VkSemaphore> renderFinishedSemaphores{};
    VkQueue graphicsQueue{};
    VkQueue presentQueue{};
    VkPipelineLayout pipelineLayout{};
    RenderPassManager renderPassManager{};

public:
    virtual void uploadAsset(Asset *asset, bool append) {
        //destroy previously created asset if any
        asset->destroy();
        //upload mesh and vertex data
        asset->vertexBuffer.setEngineLink(&renderEngineLink);
        memcpy(asset->vertexBuffer.create(sizeof(asset->vertices[0]) * asset->vertices.size(), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU), asset->vertices.data(), sizeof(asset->vertices[0]) * asset->vertices.size());
        asset->indexBuffer.setEngineLink(&renderEngineLink);
        memcpy(asset->indexBuffer.create(sizeof(asset->indices[0]) * asset->indices.size(), VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU), asset->indices.data(), sizeof(asset->indices[0]) * asset->indices.size());
        //clear textures
        asset->textureImages.clear();
        //upload textures
        stagingBuffer.destroy();
        memcpy(stagingBuffer.create(asset->width * asset->height * 4, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU), asset->textures[0], asset->width * asset->height * 4);
        asset->textureImages.resize(asset->textureImages.size() + 1);
        asset->textureImages[asset->textureImages.size() - 1].setEngineLink(&renderEngineLink);
        asset->textureImages[asset->textureImages.size() - 1].create(VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VMA_MEMORY_USAGE_GPU_ONLY, 1, asset->width, asset->height, TEXTURE, &stagingBuffer);
        //build uniform buffers
        asset->uniformBuffer.setEngineLink(&renderEngineLink);
        memcpy(asset->uniformBuffer.create(sizeof(UniformBufferObject), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU), &asset->uniformBufferObject, sizeof(UniformBufferObject));
        //build descriptor sets
        asset->descriptorSet = descriptorSetManager.createDescriptorSet({asset->uniformBuffer}, {asset->textureImages[0]}, {BUFFER, IMAGE});
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
            shaderStageInfo.stage = i % 2 ? VK_SHADER_STAGE_FRAGMENT_BIT : VK_SHADER_STAGE_VERTEX_BIT;
            shaders.push_back(shaderStageInfo);
        }
        //create graphics pipeline for this asset
        VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
        VkVertexInputBindingDescription bindingDescription = Vertex::getBindingDescription();
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = Vertex::getAttributeDescriptions();
        vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
        vertexInputStateCreateInfo.vertexAttributeDescriptionCount = attributeDescriptions.size();
        vertexInputStateCreateInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputStateCreateInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        VkPipelineInputAssemblyStateCreateInfo  inputAssemblyStateCreateInfo{VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
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
        multisampleStateCreateInfo.sampleShadingEnable = settings->msaaSamples == VK_SAMPLE_COUNT_1_BIT ? VK_FALSE : VK_TRUE;
        multisampleStateCreateInfo.rasterizationSamples = settings->msaaSamples;
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
        pipelineCreateInfo.renderPass = renderPassManager.renderPass;
        pipelineCreateInfo.subpass = 0;
        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        if (vkCreateGraphicsPipelines(device.device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &asset->graphicsPipeline) != VK_SUCCESS) { throw std::runtime_error("failed to create graphics pipeline!"); }
        for (VkPipelineShaderStageCreateInfo shader : shaders) { vkDestroyShaderModule(device.device, shader.module, nullptr); }
        asset->deletionQueue.emplace_front([&](Asset thisAsset){ vkDestroyPipeline(device.device, thisAsset.graphicsPipeline, nullptr); thisAsset.graphicsPipeline = VK_NULL_HANDLE; });
        if (append) { assets.push_back(asset); }
    }

    void updateSettings(Settings newSettings, bool updateAll) {
        if (!settings->fullscreen & newSettings.fullscreen) { glfwSetWindowMonitor(window, nullptr, 0, 0, newSettings.defaultMonitorResolution[0], newSettings.defaultMonitorResolution[1], newSettings.refreshRate); }
        else if (settings->fullscreen & !newSettings.fullscreen) {
            glfwSetWindowMonitor(window, newSettings.monitor, 0, 0, newSettings.defaultMonitorResolution[0], newSettings.defaultMonitorResolution[1], GLFW_DONT_CARE);
            glfwSetWindowMonitor(window, nullptr, 0, 0, newSettings.defaultWindowResolution[0], newSettings.defaultWindowResolution[1], GLFW_DONT_CARE);
        }
        else { glfwSetWindowSize(window, settings->resolution[0], settings->resolution[1]); }
        glfwSetWindowTitle(window, newSettings.applicationName.c_str());
        settings = &newSettings;
        if (updateAll) { createSwapchain(true); }
    }

    void cleanUp() {
        for (Asset *asset : assets) { asset->destroy(); }
        for (std::function<void()>& function : recreationDeletionQueue) { function(); }
        recreationDeletionQueue.clear();
        for (std::function<void()>& function : oneTimeOptionalDeletionQueue) { function(); }
        oneTimeOptionalDeletionQueue.clear();
        for (std::function<void()>& function : engineDeletionQueue) { function(); }
        engineDeletionQueue.clear();
        #if defined _WIN32
        FreeLibrary( vulkan_library );
        #else
        dlclose( vulkan_library );
        #endif
        vulkan_library = nullptr;
    }

    Settings *settings{};
    Camera camera{};
    GLFWwindow *window{};
    std::vector<Asset *> assets{};
    CommandBufferManager commandBufferManager{};
};
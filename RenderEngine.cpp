#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "sources/stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "sources/tiny_obj_loader.h"

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>
#include <algorithm>
#include <filesystem>
#include <vector>
#include <cstdlib>
#include <set>

class Settings {
public:
    std::vector<const char*> validationLayers = {"VK_LAYER_KHRONOS_validation"};
    std::vector<const char*> requestedExtensions  = {"VK_KHR_swapchain"};
    std::string name = "RenderEngine";
    std::array<int, 3> version = {0, 0, 1};
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
    int anisotropicFilterLevel = 1;
    bool fullscreen = false;
    int refreshRate = 60;
    std::array<int, 2> resolution = {800, 600};
    #ifdef _WIN32
    #include <direct.h>
    #include <windows.h>
    char pathChar[MAX_PATH];
    GetModuleFileNameA(NULL, pathChar, MAX_PATH);
    std::string filenameAbsolute = std::string(pathChar);
    std::string absolutePath = filenameAbsolute.substr(0, filenameAbsolute.find_last_of("\\") + 1);
    #else
    std::string filenameAbsolute{std::filesystem::canonical("/proc/self/exe").u8string()};
    std::string absolutePath = (std::string)filenameAbsolute.substr(0, filenameAbsolute.find_last_of('/') + 1);
    #endif

    Settings findMaxSettings(VkPhysicalDevice physicalDevice) {
        VkPhysicalDeviceFeatures physicalDeviceFeatures{};
        vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceFeatures);
        VkPhysicalDeviceProperties physicalDeviceProperties{};
        vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        refreshRate = mode->refreshRate;
        resolution[0] = mode->width;
        resolution[1] = mode->height;
        VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts &
                                    physicalDeviceProperties.limits.framebufferDepthSampleCounts;
        if (counts & VK_SAMPLE_COUNT_64_BIT) {msaaSamples = VK_SAMPLE_COUNT_64_BIT;}
        else if (counts & VK_SAMPLE_COUNT_32_BIT) {msaaSamples = VK_SAMPLE_COUNT_32_BIT;}
        else if (counts & VK_SAMPLE_COUNT_16_BIT) {msaaSamples = VK_SAMPLE_COUNT_16_BIT;}
        else if (counts & VK_SAMPLE_COUNT_8_BIT) {msaaSamples = VK_SAMPLE_COUNT_8_BIT;}
        else if (counts & VK_SAMPLE_COUNT_4_BIT) {msaaSamples = VK_SAMPLE_COUNT_4_BIT;}
        else if (counts & VK_SAMPLE_COUNT_2_BIT) {msaaSamples = VK_SAMPLE_COUNT_2_BIT;}
        else if (counts & VK_SAMPLE_COUNT_1_BIT) {msaaSamples = VK_SAMPLE_COUNT_1_BIT;}
        if (physicalDeviceFeatures.samplerAnisotropy) {
            anisotropicFilterLevel = static_cast<int>(physicalDeviceProperties.limits.maxSamplerAnisotropy);
        }
        return *this;
    }
};

class VulkanRenderEngine {
public:
    GLFWwindow* window{};
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    explicit VulkanRenderEngine(const Settings& startSettings) {
        settings = startSettings;
        settings.fullscreen = false;
        createWindow();
        setSettings(startSettings, false);
        createVulkanInstance();
        createWindowSurface();
        findBestSuitableDevice();
        createLogicalDevice();
        createSwapChain();
        createImageViews();
        createRenderPass();
        createDescriptorSetLayout();
        createGraphicsPipeline();
    }

    void start() {

    }

    void setSettings(const Settings& newSettings, bool updateAll) {
        if (!settings.fullscreen & newSettings.fullscreen) {glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), 0, 0, newSettings.resolution[0], newSettings.resolution[0], newSettings.refreshRate);}
        else if (settings.fullscreen & !newSettings.fullscreen) {glfwSetWindowMonitor(window, nullptr, 0, 0, 800, 600, newSettings.refreshRate);}
        else {glfwSetWindowSize(window, newSettings.resolution[0], newSettings.resolution[1]);}
        settings = newSettings;
        if (updateAll) {
            createRenderPass();
            createGraphicsPipeline();
        }
    }

    int update() {
        if (glfwWindowShouldClose(window)) {
            cleanUp();
            vkDeviceWaitIdle(logicalDevice);
            return 1;}
        return 0;
    }
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

    struct Vertex {
        glm::vec3 pos;
        glm::vec3 color;
        glm::vec2 texCoord;

        static VkVertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription bindingDescription;
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);
            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);
            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

            return attributeDescriptions;
        }

        bool operator==(const Vertex& other) const {
            return pos == other.pos && color == other.color && texCoord == other.texCoord;
        }
    };

    VkInstance instance{};
    VkDebugUtilsMessengerEXT debugMessenger{};
    bool framebufferResized = false;
    VkSurfaceKHR surface{};
    VkPhysicalDeviceProperties physicalDeviceProperties{};
    VkPhysicalDeviceFeatures physicalDeviceFeatures{};
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties{};
    Settings settings{};
    QueueFamilyIndices indices{};
    VkQueue graphicsQueue{};
    VkQueue presentQueue{};
    VkDevice logicalDevice{};
    SwapChainSupportDetails swapChainSupport;
    VkSwapchainKHR swapChain{};
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat{};
    VkExtent2D swapChainExtent{};
    std::vector<VkImageView> swapChainImageViews;
    VkRenderPass renderPass{};
    VkDescriptorSetLayout descriptorSetLayout{};
    VkPipelineLayout pipelineLayout{};
    VkPipeline graphicsPipeline{};

    void cleanUp() const {
        glfwSetWindowMonitor(window, nullptr, 0, 0, 800, 600, 0);
    }

    VkPipeline createGraphicsPipeline() {
        VkShaderModule vertShaderModule = createShaderModule(readFile("shaders/vert.spv"));
        VkShaderModule fragShaderModule = createShaderModule(readFile("shaders/frag.spv"));
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
        multisampling.sampleShadingEnable = VK_FALSE;
        multisampling.rasterizationSamples = settings.msaaSamples;
        multisampling.sampleShadingEnable = VK_TRUE;
        multisampling.minSampleShading = 0.2f;
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
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkFormat depthFormat;
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
        std::array<VkAttachmentDescription, 3> attachments = {colorAttachment, depthAttachment, colorAttachmentResolve};
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        renderPassInfo.pAttachments = attachments.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = 1;
        renderPassInfo.pDependencies = &dependency;
        if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {throw std::runtime_error("failed to create render pass!");}
        return renderPass;
    }

    std::vector<VkImageView> createImageViews() {
        swapChainImageViews.resize(swapChainImages.size());
        for (size_t i = 0; i < swapChainImages.size(); i++) {swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1);}
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
        //Create swap chian
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
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
        if (indices.graphicsFamily != indices.presentFamily) {
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
        vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(logicalDevice, swapChain, &imageCount, swapChainImages.data());
        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
        return swapChain;
    }

    VkDevice createLogicalDevice() {
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());
        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {indices.graphicsFamily = i;}
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
            if (presentSupport) {indices.presentFamily = i;}
            if (indices.presentFamily.has_value() && indices.graphicsFamily.has_value()) {break;}
            i++;
        }
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }
        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = (settings.anisotropicFilterLevel > 1) ? VK_TRUE: VK_FALSE;
        deviceFeatures.sampleRateShading = VK_TRUE;
        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(settings.requestedExtensions.size());
        createInfo.ppEnabledExtensionNames = settings.requestedExtensions.data();
        if (!settings.validationLayers.empty()) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(settings.validationLayers.size());
            createInfo.ppEnabledLayerNames = settings.validationLayers.data();
        } else {createInfo.enabledLayerCount = 0;}
        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logicalDevice) != VK_SUCCESS) {throw std::runtime_error("failed to create logical device!");}
        vkGetDeviceQueue(logicalDevice, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(logicalDevice, indices.presentFamily.value(), 0, &presentQueue);
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
        appInfo.pApplicationName = settings.name.c_str();
        appInfo.applicationVersion = VK_MAKE_VERSION(settings.version[0], settings.version[1], settings.version[2]);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
        appInfo.apiVersion = VK_API_VERSION_1_2;
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
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
            for (const char* layerName : settings.validationLayers) {
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
        if (CreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, &debugMessenger) != VK_SUCCESS) {throw std::runtime_error("failed to setup debug messenger!");}
        return instance;
    }

    GLFWwindow* createWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window = glfwCreateWindow(settings.resolution[0], settings.resolution[1], settings.name.c_str(), nullptr, nullptr);
        glfwSetWindowUserPointer(window, this);
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        return window;
    }

    VkShaderModule createShaderModule(const std::vector<char>& code) {
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
        VkShaderModule shaderModule;
        if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {throw std::runtime_error("failed to create shader module!");}
        return shaderModule;
    }

    std::vector<char> readFile(const char* filename) {
        std::string filenameAbsolute = settings.absolutePath + filename;
        std::ifstream file(filenameAbsolute, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {throw std::runtime_error("failed to open file: " + std::string(filename) + "\n as file: " + filenameAbsolute);}
        size_t fileSize = (size_t) file.tellg();
        std::vector<char> buffer(fileSize);
        file.seekg(0);
        file.read(buffer.data(), fileSize);
        file.close();
        return buffer;
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
        VkImageView imageView;
        if (vkCreateImageView(logicalDevice, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {throw std::runtime_error("failed to create image view!");}
        return imageView;
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        reinterpret_cast<VulkanRenderEngine*>(glfwGetWindowUserPointer(window))->framebufferResized = true;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {return func(instance, pCreateInfo, pAllocator, pDebugMessenger);}
        else {return VK_ERROR_EXTENSION_NOT_PRESENT;}
    }
};

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {glfwSetWindowShouldClose(window, 1);}
}

int main() {
    Settings settings{};
    VulkanRenderEngine RenderEngine(settings);
    settings.fullscreen = true;
    RenderEngine.setSettings(settings.findMaxSettings(RenderEngine.physicalDevice), true);
    try {
        RenderEngine.start();
        glfwSetKeyCallback(RenderEngine.window, keyCallback);
        while (RenderEngine.update() != 1) {
            glfwPollEvents();
        }
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
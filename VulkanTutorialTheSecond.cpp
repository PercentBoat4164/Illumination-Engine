#define GLFW_INCLUDE_VULKAN
#define GLSLC "glslc "

#include <GLFW/glfw3.h>

#include <VkBootstrap.h>
#include <cmath>
#include <vector>
#include <fstream>
#include <array>
#include <deque>
#include <functional>


struct DeletionQueue {
    std::deque<std::function<void()>> deletors;

    void push_function(std::function<void()>&& function) {deletors.push_back(function);}

    void flush() {
        for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {(*it)();}
        deletors.clear();
    }
};

class VulkanEngine {
public:
    bool _isInitialized{false};
    int _frameNumber{0};
    VkExtent2D _windowExtent{1280, 720};
    GLFWwindow *_window;
    VkInstance _instance;
    VkDebugUtilsMessengerEXT _debug_messenger;
    VkPhysicalDevice _chosenGPU;
    VkDevice _device;
    DeletionQueue _mainDeletionQueue{};
    VkSurfaceKHR _surface{};
    VkSwapchainKHR _swapchain{};
    VkFormat _swapchainImageFormat{};
    std::vector<VkImage> _swapchainImages;
    std::vector<VkImageView> _swapchainImageViews;
    VkQueue _graphicsQueue;
    uint32_t _graphicsQueueFamily;
    VkCommandPool _commandPool{};
    VkCommandBuffer _mainCommandBuffer{};
    VkRenderPass _renderPass{};
    std::vector<VkFramebuffer> _framebuffers;
    VkSemaphore _presentSemaphore{}, _renderSemaphore{};
    VkFence _renderFence{};
    VkPipelineLayout _trianglePipelineLayout{};
    VkViewport _viewport{};
    VkRect2D _scissor{};
    VkPipeline _trianglePipeline{};
    bool RGBTri{true};
    bool framebufferResized{false};

    VulkanEngine() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        _window = glfwCreateWindow(_windowExtent.width, _windowExtent.height, "Vulkan Tutorial", nullptr, nullptr);
        glfwSetWindowUserPointer(_window, this);
        glfwSetFramebufferSizeCallback(_window, FramebufferResizeCallback);
        vkb::InstanceBuilder builder;
        auto inst_ret = builder.set_app_name("Example Vulkan Application").request_validation_layers(true).require_api_version(1, 2, 0).use_default_debug_messenger().build();
        vkb::Instance vkb_inst = inst_ret.value();
        _instance = vkb_inst.instance;
        _debug_messenger = vkb_inst.debug_messenger;
        if (glfwCreateWindowSurface(_instance, _window, nullptr, &_surface) != VK_SUCCESS) {throw std::runtime_error("failed to create window surface!");}
        vkb::PhysicalDeviceSelector selector{vkb_inst};
        vkb::PhysicalDevice physicalDevice = selector.set_surface(_surface).select().value();
        vkb::DeviceBuilder deviceBuilder{physicalDevice};
        vkb::Device vkbDevice = deviceBuilder.build().value();
        _device = vkbDevice.device;
        _chosenGPU = physicalDevice.physical_device;
        _graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
        _graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();
    }

    void buildEngine(std::array<std::string, 2> shaderPaths) {
        vkb::SwapchainBuilder swapchainBuilder{_chosenGPU, _device, _surface};
        vkb::Swapchain vkbSwapchain = swapchainBuilder.use_default_format_selection().set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR).set_desired_extent(_windowExtent.width, _windowExtent.height).build().value();
        _swapchain = vkbSwapchain.swapchain;
        _swapchainImages = vkbSwapchain.get_images().value();
        _swapchainImageViews = vkbSwapchain.get_image_views().value();
        _swapchainImageFormat = vkbSwapchain.image_format;
        _mainDeletionQueue.push_function([=]() {vkDestroySwapchainKHR(_device, _swapchain, nullptr);});
        VkCommandPoolCreateInfo commandPoolInfo{};
        commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolInfo.pNext = nullptr;
        commandPoolInfo.queueFamilyIndex = _graphicsQueueFamily;
        commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        if (vkCreateCommandPool(_device, &commandPoolInfo, nullptr, &_commandPool) != VK_SUCCESS) {throw std::runtime_error("failed to create command pool!");}
        VkCommandBufferAllocateInfo cmdAllocInfo{};
        cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdAllocInfo.pNext = nullptr;
        cmdAllocInfo.commandPool = _commandPool;
        cmdAllocInfo.commandBufferCount = 1;
        cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        if(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &_mainCommandBuffer) != VK_SUCCESS) {throw std::runtime_error("failed to allocate command buffers!");}
        _mainDeletionQueue.push_function([=]() {vkDestroyCommandPool(_device, _commandPool, nullptr);});
        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = _swapchainImageFormat;
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT; //Only thing that changes with number of msaa samples.
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &colorAttachmentRef;
        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &colorAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        if (vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderPass) != VK_SUCCESS) {throw std::runtime_error("failed to create the render pass!");}
        _mainDeletionQueue.push_function([=]() {vkDestroyRenderPass(_device, _renderPass, nullptr);});
        VkFramebufferCreateInfo fbInfo{};
        fbInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        fbInfo.pNext = nullptr;
        fbInfo.renderPass = _renderPass;
        fbInfo.attachmentCount = 1;
        fbInfo.width = _windowExtent.width;
        fbInfo.height = _windowExtent.height;
        fbInfo.layers = 1;
        const uint32_t swapchainImageCount = _swapchainImages.size();
        _framebuffers.resize(swapchainImageCount);
        for (int i = 0; i < swapchainImageCount; i++) {
            fbInfo.pAttachments = &_swapchainImageViews[i];
            if (vkCreateFramebuffer(_device, &fbInfo, nullptr, &_framebuffers[i]) != VK_SUCCESS) {throw std::runtime_error("failed to create framebuffers!");}
            _mainDeletionQueue.push_function([=]() {
                vkDestroyFramebuffer(_device, _framebuffers[i], nullptr);
                vkDestroyImageView(_device, _swapchainImageViews[i], nullptr);
            });
        }
        VkFenceCreateInfo fenceCreateInfo{};
        fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceCreateInfo.pNext = nullptr;
        fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        if (vkCreateFence(_device, &fenceCreateInfo, nullptr, &_renderFence) != VK_SUCCESS) {throw std::runtime_error("failed to create fence!");}
        _mainDeletionQueue.push_function([=]() {vkDestroyFence(_device, _renderFence, nullptr);});
        VkSemaphoreCreateInfo semaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        semaphoreCreateInfo.pNext = nullptr;
        semaphoreCreateInfo.flags = 0;
        if (vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_presentSemaphore) != VK_SUCCESS) {throw std::runtime_error("failed to create present semaphore!");}
        _mainDeletionQueue.push_function([=]() {vkDestroySemaphore(_device, _presentSemaphore, nullptr);});
        if (vkCreateSemaphore(_device, &semaphoreCreateInfo, nullptr, &_renderSemaphore) != VK_SUCCESS) {throw std::runtime_error("failed to create render semaphore!");}
        _mainDeletionQueue.push_function([=]() {vkDestroySemaphore(_device, _renderSemaphore, nullptr);});
        VkShaderModule triangleFragShader = loadShaderModule(shaderPaths[0].c_str());
        VkShaderModule triangleVertShader = loadShaderModule(shaderPaths[1].c_str());
        VkPipelineShaderStageCreateInfo fragmentShaderInfo{};
        fragmentShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        fragmentShaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        fragmentShaderInfo.module = triangleFragShader;
        fragmentShaderInfo.pName = "main";
        VkPipelineShaderStageCreateInfo vertexShaderInfo{};
        vertexShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vertexShaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vertexShaderInfo.module = triangleVertShader;
        vertexShaderInfo.pName = "main";
        VkPipelineVertexInputStateCreateInfo vertexInputCreateInfo{};
        vertexInputCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputCreateInfo.vertexBindingDescriptionCount = 0;
        vertexInputCreateInfo.vertexAttributeDescriptionCount = 0;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
        inputAssemblyCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssemblyCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;
        VkPipelineRasterizationStateCreateInfo rasterizationCreateInfo{};
        rasterizationCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizationCreateInfo.rasterizerDiscardEnable = VK_FALSE;
        rasterizationCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
        rasterizationCreateInfo.lineWidth = 1.f;
        rasterizationCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT; //Enable backface culling
        rasterizationCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizationCreateInfo.depthBiasEnable = VK_FALSE; //Use this for banding in shadows?
        rasterizationCreateInfo.depthBiasConstantFactor = 0.f;
        rasterizationCreateInfo.depthBiasClamp = 0.f;
        rasterizationCreateInfo.depthBiasSlopeFactor = 0.f;
        VkPipelineMultisampleStateCreateInfo multisampleCreateInfo{}; //Change this and more to add optional msaa
        multisampleCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleCreateInfo.sampleShadingEnable = VK_FALSE;
        multisampleCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleCreateInfo.minSampleShading = 1.f;
        multisampleCreateInfo.pSampleMask = nullptr;
        multisampleCreateInfo.alphaToCoverageEnable = VK_FALSE;
        multisampleCreateInfo.alphaToOneEnable = VK_FALSE;
        VkPipelineColorBlendAttachmentState colorAttachmentCreateInfo{};
        colorAttachmentCreateInfo.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorAttachmentCreateInfo.blendEnable = VK_FALSE;
        VkPipelineViewportStateCreateInfo viewportCreateInfo{};
        viewportCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportCreateInfo.viewportCount = 1;
        viewportCreateInfo.pViewports = &_viewport;
        viewportCreateInfo.scissorCount = 1;
        viewportCreateInfo.pScissors = &_scissor;
        _viewport.x = 0.f;
        _viewport.y = 0.f;
        _viewport.width = (float)_windowExtent.width;
        _viewport.height = (float)_windowExtent.height;
        _viewport.minDepth = 0.f;
        _viewport.maxDepth = 1.f;
        _scissor.offset = {0, 0};
        _scissor.extent = _windowExtent;
        VkPipelineColorBlendStateCreateInfo colorBlendCreateInfo{};
        colorBlendCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendCreateInfo.logicOpEnable = VK_FALSE;
        colorBlendCreateInfo.logicOp = VK_LOGIC_OP_COPY;
        colorBlendCreateInfo.attachmentCount = 1;
        colorBlendCreateInfo.pAttachments = &colorAttachmentCreateInfo;
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        if (vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_trianglePipelineLayout) != VK_SUCCESS) {throw std::runtime_error("failed to create pipeline layout!");}
        _mainDeletionQueue.push_function([=]() {vkDestroyPipelineLayout(_device, _trianglePipelineLayout, nullptr);});
        VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo{};
        graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        graphicsPipelineCreateInfo.stageCount = 2;
        graphicsPipelineCreateInfo.pStages = std::array<VkPipelineShaderStageCreateInfo, 2>({fragmentShaderInfo, vertexShaderInfo}).data();
        graphicsPipelineCreateInfo.pVertexInputState = &vertexInputCreateInfo;
        graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
        graphicsPipelineCreateInfo.pViewportState = &viewportCreateInfo;
        graphicsPipelineCreateInfo.pRasterizationState = &rasterizationCreateInfo;
        graphicsPipelineCreateInfo.pMultisampleState = &multisampleCreateInfo;
        graphicsPipelineCreateInfo.pColorBlendState = &colorBlendCreateInfo;
        graphicsPipelineCreateInfo.layout = _trianglePipelineLayout;
        graphicsPipelineCreateInfo.renderPass = _renderPass;
        graphicsPipelineCreateInfo.subpass = 0;
        graphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;
        if (vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &_trianglePipeline) != VK_SUCCESS) {throw std::runtime_error("failed to create graphics pipelines!");}
        _mainDeletionQueue.push_function([=]() {vkDestroyPipeline(_device, _trianglePipeline, nullptr);});
        vkDestroyShaderModule(_device, triangleFragShader, nullptr);
        vkDestroyShaderModule(_device, triangleVertShader, nullptr);
        _isInitialized = true;
    }

    void cleanUp() {
        if (_isInitialized) {
            vkWaitForFences(_device, 1, &_renderFence, true, 1000000000);
            _mainDeletionQueue.flush();
            vkDestroyDevice(_device, nullptr);
            vkDestroySurfaceKHR(_instance, _surface, nullptr);
            vkb::destroy_debug_utils_messenger(_instance, _debug_messenger);
            vkDestroyInstance(_instance, nullptr);
            glfwDestroyWindow(_window);
        }
    }

    int update() {
        if (glfwWindowShouldClose(_window)) {
            cleanUp();
            return 1;
        }
        vkWaitForFences(_device, 1, &_renderFence, true, 1000000000);
        vkResetFences(_device, 1, &_renderFence);
        uint32_t swapchainImageIndex;
        vkAcquireNextImageKHR(_device, _swapchain, 1000000000, _presentSemaphore, nullptr, &swapchainImageIndex);
        vkResetCommandBuffer(_mainCommandBuffer, 0);
        VkCommandBufferBeginInfo cmdBeginInfo{};
        cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        cmdBeginInfo.pNext = nullptr;
        cmdBeginInfo.pInheritanceInfo = nullptr;
        cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(_mainCommandBuffer, &cmdBeginInfo);
        VkClearValue clearValue;
        float speed = 20;
        speed = 1000/speed;
        float red = std::sin((float)(_frameNumber + (2 * M_PI / 3 * speed)) / speed);
        float green = std::sin((float)(_frameNumber) / speed);
        float blue = std::sin((float)(_frameNumber + (4 * M_PI / 3 * speed)) / speed);
        clearValue.color = {{red, green, blue, 1.f}};
        VkRenderPassBeginInfo rpInfo{};
        rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpInfo.pNext = nullptr;
        rpInfo.renderPass = _renderPass;
        rpInfo.renderArea.offset.x = 0;
        rpInfo.renderArea.offset.y = 0;
        rpInfo.renderArea.extent = _windowExtent;
        rpInfo.framebuffer = _framebuffers[swapchainImageIndex];
        rpInfo.clearValueCount = 1;
        rpInfo.pClearValues = &clearValue;
        vkCmdBeginRenderPass(_mainCommandBuffer, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(_mainCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _trianglePipeline);
        vkCmdDraw(_mainCommandBuffer, 3, 1, 0, 0);
        vkCmdEndRenderPass(_mainCommandBuffer);
        vkEndCommandBuffer(_mainCommandBuffer);
        VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        VkSubmitInfo submit{};
        submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit.pWaitDstStageMask = &waitStage;
        submit.waitSemaphoreCount = 1;
        submit.pWaitSemaphores = &_presentSemaphore;
        submit.signalSemaphoreCount = 1;
        submit.pSignalSemaphores = &_renderSemaphore;
        submit.commandBufferCount = 1;
        submit.pCommandBuffers = &_mainCommandBuffer;
        vkQueueSubmit(_graphicsQueue, 1, &submit, _renderFence);
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.pSwapchains = &_swapchain;
        presentInfo.swapchainCount = 1;
        presentInfo.pWaitSemaphores = &_renderSemaphore;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pImageIndices = &swapchainImageIndex;
        vkQueuePresentKHR(_graphicsQueue, &presentInfo);
        if (framebufferResized) {
            framebufferResized = false;
            buildEngine({"shaders/VulkanTutorialTheSecondFragmentShader.frag", "shaders/VulkanTutorialTheSecondVertexShader.vert"});
        }
        _frameNumber++;
        glfwPollEvents();
        return 0;
    }

    void toggleShaders() {
        if (RGBTri) {buildEngine({"shaders/coloredTriFrag.frag", "shaders/coloredTriVert.vert"});}
        else {buildEngine({"shaders/VulkanTutorialTheSecondFragmentShader.frag", "shaders/VulkanTutorialTheSecondVertexShader.vert"});}
        RGBTri ^= true;
    }

private:
    static void FramebufferResizeCallback(GLFWwindow *window, int width, int height) {
        reinterpret_cast<VulkanEngine *>(glfwGetWindowUserPointer(window))->framebufferResized = true;
    }

    VkShaderModule loadShaderModule(const char* filePath) const {
        std::string path = (std::string)"/home/thaddeus/Documents/Programming/C++/GameEngine/cmake-build-debug/" + filePath;
        system((GLSLC + path + " -o " + path.substr(0, path.find_last_of('.')) + ".spv").c_str());
        path = path.substr(0, path.find_last_of('.')) + ".spv";
        std::ifstream file(path, std::ios::ate | std::ios::binary);
        if (!file.is_open()) {throw std::runtime_error("failed to open file: " + path + "\n as file: " + path);}
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
        if (vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {throw std::runtime_error("failed to create shader module!");}
        return shaderModule;
    }
};

void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_SPACE & action == GLFW_PRESS) {
        auto *renderEngine = static_cast<VulkanEngine *>(glfwGetWindowUserPointer(window));
        renderEngine->toggleShaders();
    }
}

int main() {
    VulkanEngine renderEngine;
    renderEngine.buildEngine({"shaders/VulkanTutorialTheSecondFragmentShader.frag", "shaders/VulkanTutorialTheSecondVertexShader.vert"});
    glfwSetKeyCallback(renderEngine._window, keyCallback);
    while (renderEngine.update() != 1) {
        //GameLoop
    }
}
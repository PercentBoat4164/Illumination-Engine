#pragma once

#include "vulkanRenderPass.hpp"

class VulkanFramebuffer {
public:
    struct CreateInfo {
        //Required
        VulkanRenderPass renderPass{};
        VkImageView swapchainImageView{};
    };

    CreateInfo createdWith{};
    VkFramebuffer framebuffer{};
    std::vector<VkClearValue> clearValues{};
    VulkanGraphicsEngineLink *linkedRenderEngine{};
    VulkanImage colorImage{};
    VulkanImage depthImage{};

    void create(VulkanGraphicsEngineLink *engineLink, CreateInfo *createInfo) {
        createdWith = *createInfo;
        linkedRenderEngine = engineLink;
        if (createdWith.swapchainImageView == VK_NULL_HANDLE) { throw std::runtime_error("VulkanFramebuffer::CreateInfo::swapchainImageView cannot be VK_NULL_HANDLE!"); }
        if (createdWith.renderPass.renderPass == VK_NULL_HANDLE) { throw std::runtime_error("VulkanFramebuffer::CreateInfo::renderPass::renderPass cannot be VK_NULL_HANDLE!"); }
        clearValues.resize(linkedRenderEngine->settings->msaaSamples == VK_SAMPLE_COUNT_1_BIT ? 2 : 3);
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        VulkanImage::CreateInfo framebufferImageCreateInfo{linkedRenderEngine->swapchain->image_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY};
        framebufferImageCreateInfo.msaaSamples = linkedRenderEngine->settings->msaaSamples;
        if (linkedRenderEngine->settings->msaaSamples != VK_SAMPLE_COUNT_1_BIT) {
            clearValues[2].color = clearValues[0].color;
            framebufferImageCreateInfo.imageType = VULKAN_COLOR;
            colorImage.create(linkedRenderEngine, &framebufferImageCreateInfo);
            deletionQueue.emplace_back([&] { colorImage.destroy(); });
        }
        framebufferImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        framebufferImageCreateInfo.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        framebufferImageCreateInfo.imageType = VULKAN_DEPTH;
        framebufferImageCreateInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depthImage.create(linkedRenderEngine, &framebufferImageCreateInfo);
        deletionQueue.emplace_back([&] { depthImage.destroy(); });
        std::vector<VkImageView> framebufferAttachments{linkedRenderEngine->settings->msaaSamples == VK_SAMPLE_COUNT_1_BIT ? createdWith.swapchainImageView : colorImage.view, depthImage.view, createdWith.swapchainImageView};
        VkFramebufferCreateInfo framebufferCreateInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        framebufferCreateInfo.renderPass = createdWith.renderPass.renderPass;
        framebufferCreateInfo.attachmentCount = linkedRenderEngine->settings->msaaSamples == VK_SAMPLE_COUNT_1_BIT ? 2 : 3;
        framebufferCreateInfo.pAttachments = framebufferAttachments.data();
        framebufferCreateInfo.width = linkedRenderEngine->swapchain->extent.width;
        framebufferCreateInfo.height = linkedRenderEngine->swapchain->extent.height;
        framebufferCreateInfo.layers = 1;
        if (vkCreateFramebuffer(linkedRenderEngine->device->device, &framebufferCreateInfo, nullptr, &framebuffer) != VK_SUCCESS) { throw std::runtime_error("failed to create framebuffers!"); }
        deletionQueue.emplace_back([&] { vkDestroyFramebuffer(linkedRenderEngine->device->device, framebuffer, nullptr); });
    }

    void destroy() {
        for (std::function<void()> &function : deletionQueue) { function(); }
        deletionQueue.clear();
    }

private:
    std::deque<std::function<void()>> deletionQueue{};
};

VkRenderPassBeginInfo VulkanRenderPass::beginRenderPass(const VulkanFramebuffer& framebuffer) {
    VkRenderPassBeginInfo renderPassBeginInfo{VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
    renderPassBeginInfo.renderArea.offset = {0, 0};
    renderPassBeginInfo.renderArea.extent = linkedRenderEngine->swapchain->extent;
    renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(framebuffer.clearValues.size());
    renderPassBeginInfo.pClearValues = framebuffer.clearValues.data();
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = framebuffer.framebuffer;
    return renderPassBeginInfo;
}
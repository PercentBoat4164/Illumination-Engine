#pragma once

#include "GraphicsModule/VulkanOpenGL/IEGraphicsLink.hpp"
#include "IEImage.hpp"

#include <vulkan/vulkan.h>
#include <vector>

class IEFramebuffer {
public:
    struct CreateInfo {
        // Required
        uint8_t msaaSamples{1};
    };

    CreateInfo createdWith{};
    VkFramebuffer framebuffer{};
    std::vector<VkClearValue> clearValues{3};
    IEGraphicsLink* linkedRenderEngine{};
    IEImage colorImage{};
    IEImage depthImage{};

    void create(IEGraphicsLink *engineLink, VkImageView swapchainImageView, VkRenderPass renderPass) {
        linkedRenderEngine = engineLink;
        if (swapchainImageView == VK_NULL_HANDLE) {
            throw std::runtime_error("IEFramebuffer::CreateInfo::swapchainImageView cannot be VK_NULL_HANDLE!");
        }
        if (renderPass == VK_NULL_HANDLE) {
            throw std::runtime_error("IEFramebuffer::CreateInfo::renderPass::renderPass cannot be VK_NULL_HANDLE!");
        }
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        clearValues[2].color = clearValues[0].color;
        IEImage::CreateInfo framebufferImageCreateInfo{linkedRenderEngine->swapchain.image_format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY};
        VkImageFormatProperties imageFormats;
        vkGetPhysicalDeviceImageFormatProperties(linkedRenderEngine->device.physical_device.physical_device, framebufferImageCreateInfo.format, VK_IMAGE_TYPE_2D, framebufferImageCreateInfo.tiling, framebufferImageCreateInfo.usage, VK_IMAGE_CREATE_EXTENDED_USAGE_BIT_KHR, &imageFormats);
        framebufferImageCreateInfo.msaaSamples = static_cast<VkSampleCountFlagBits>(std::min(static_cast<uint32_t>(linkedRenderEngine->settings.msaaSamples), imageFormats.sampleCounts));
        if (linkedRenderEngine->settings.msaaSamples != VK_SAMPLE_COUNT_1_BIT) {
            colorImage.create(linkedRenderEngine, &framebufferImageCreateInfo);
            deletionQueue.emplace_back([&] {
                colorImage.destroy();
            });
        }
        framebufferImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        framebufferImageCreateInfo.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        framebufferImageCreateInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depthImage.create(linkedRenderEngine, &framebufferImageCreateInfo);
        deletionQueue.emplace_back([&] {
            depthImage.destroy();
        });
        std::vector<VkImageView> framebufferAttachments{linkedRenderEngine->settings.msaaSamples == VK_SAMPLE_COUNT_1_BIT ? swapchainImageView : colorImage.view, depthImage.view, swapchainImageView};
        VkFramebufferCreateInfo framebufferCreateInfo{VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO};
        framebufferCreateInfo.renderPass = renderPass;
        framebufferCreateInfo.attachmentCount = linkedRenderEngine->settings.msaaSamples == VK_SAMPLE_COUNT_1_BIT ? 2 : 3;
        framebufferCreateInfo.pAttachments = framebufferAttachments.data();
        framebufferCreateInfo.width = linkedRenderEngine->swapchain.extent.width;
        framebufferCreateInfo.height = linkedRenderEngine->swapchain.extent.height;
        framebufferCreateInfo.layers = 1;
        if (vkCreateFramebuffer(linkedRenderEngine->device.device, &framebufferCreateInfo, nullptr, &framebuffer) != VK_SUCCESS) { throw std::runtime_error("failed to create framebuffers!"); }
        deletionQueue.emplace_back([&] { vkDestroyFramebuffer(linkedRenderEngine->device.device, framebuffer, nullptr); });
    }

    void destroy() {
        for (std::function<void()> &function : deletionQueue) {
            function();
        }
        deletionQueue.clear();
    }

    ~IEFramebuffer() {
        destroy();
    }

private:
    std::vector<std::function<void()>> deletionQueue{};
};
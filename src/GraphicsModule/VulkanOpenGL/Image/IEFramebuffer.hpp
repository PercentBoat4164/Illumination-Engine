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
        VkImageView swapchainImageView{};
        VkRenderPass renderPass{};
    };

    CreateInfo createdWith{};
    VkFramebuffer framebuffer{};
    std::vector<VkClearValue> clearValues{3};
    IEGraphicsLink* linkedRenderEngine{};
    IEImage colorImage{};
    IEImage depthImage{};

    void create(IEGraphicsLink *engineLink, CreateInfo *createInfo) {
        linkedRenderEngine = engineLink;
        createdWith = *createInfo;
        if (createdWith.swapchainImageView == VK_NULL_HANDLE) {
            throw std::runtime_error("IEFramebuffer::CreateInfo::swapchainImageView cannot be VK_NULL_HANDLE!");
        }
        if (createdWith.renderPass == nullptr) {
            throw std::runtime_error("IEFramebuffer::CreateInfo::renderPass cannot be a nullptr!");
        }
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {1.0f, 0};
        clearValues[2].color = clearValues[0].color;

        // Verify number of allowed msaa samples
        uint8_t msaaSamplesAllowed = std::min(linkedRenderEngine->settings.msaaSamples, createdWith.msaaSamples);

        // Create framebuffer images
        IEImage::CreateInfo framebufferImageCreateInfo{
            .format=linkedRenderEngine->swapchain.image_format,
            .tiling=VK_IMAGE_TILING_OPTIMAL,
            .usage=VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .allocationUsage=VMA_MEMORY_USAGE_GPU_ONLY
        };
        // Check that the number of samples is supported by the format.
        VkImageFormatProperties imageFormats;
        vkGetPhysicalDeviceImageFormatProperties(linkedRenderEngine->device.physical_device.physical_device, framebufferImageCreateInfo.format, VK_IMAGE_TYPE_2D, framebufferImageCreateInfo.tiling, framebufferImageCreateInfo.usage, VK_IMAGE_CREATE_EXTENDED_USAGE_BIT_KHR, &imageFormats);
        framebufferImageCreateInfo.msaaSamples = static_cast<VkSampleCountFlagBits>(std::min(msaaSamplesAllowed, static_cast<uint8_t>(imageFormats.sampleCounts)));
        if (msaaSamplesAllowed > VK_SAMPLE_COUNT_1_BIT) {
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
        std::vector<VkImageView> framebufferAttachments{msaaSamplesAllowed == VK_SAMPLE_COUNT_1_BIT ? createdWith.swapchainImageView : colorImage.view, depthImage.view, createdWith.swapchainImageView};
        VkFramebufferCreateInfo framebufferCreateInfo{
                .sType=VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = createdWith.renderPass,
                .attachmentCount=msaaSamplesAllowed == VK_SAMPLE_COUNT_1_BIT ? 2u : 3u,
                .pAttachments=framebufferAttachments.data(),
                .width=linkedRenderEngine->swapchain.extent.width,
                .height=linkedRenderEngine->swapchain.extent.height,
                .layers=1
        };
        if (vkCreateFramebuffer(linkedRenderEngine->device.device, &framebufferCreateInfo, nullptr, &framebuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffers!");
        }
        deletionQueue.emplace_back([&] {
            vkDestroyFramebuffer(linkedRenderEngine->device.device, framebuffer, nullptr);
        });
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
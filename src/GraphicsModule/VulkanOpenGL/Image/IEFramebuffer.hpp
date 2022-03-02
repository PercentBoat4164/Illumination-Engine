#pragma once

#include "GraphicsModule/VulkanOpenGL/IEGraphicsLink.hpp"
#include "IEImage.hpp"

#include <vulkan/vulkan.h>
#include <vector>

class IEFramebuffer : public IEImage {
public:
    struct CreateInfo {
        VkImageView swapchainImageView{};
        VkRenderPass renderPass{};
        std::vector<float> defaultColor{0.0f, 0.0f, 0.0f, 1.0f};
    };

    VkFramebuffer framebuffer{};
    std::vector<VkClearValue> clearValues{3};
    std::vector<float> defaultColor{0.0f, 0.0f, 0.0f, 1.0f};
    VkImageView swapchainImageView{};
    VkRenderPass renderPass{};
    IEImage colorImage{};
    IEImage depthImage{};

    virtual void copyCreateInfo(IEFramebuffer::CreateInfo *createInfo) {
        swapchainImageView = createInfo->swapchainImageView;
        renderPass = createInfo->renderPass;
        defaultColor = createInfo->defaultColor;
    }

    void create(IEGraphicsLink *engineLink, CreateInfo *createInfo) {
        if (engineLink) {  // Assume that this image is being recreated in a new engine, or created for the first time.
            destroy();  // Delete anything that was created in the context of the old engine
            linkedRenderEngine = engineLink;
        }

        // Copy createInfo data into this image
        copyCreateInfo(createInfo);

        // Verify that width and height are suitable
        width = width == 0 ? static_cast<uint16_t>(linkedRenderEngine->swapchain.extent.width) : width;
        height = height == 0 ? static_cast<uint16_t>(linkedRenderEngine->swapchain.extent.height) : height;

        if (swapchainImageView == VK_NULL_HANDLE) {
            throw std::runtime_error("IEFramebuffer::CreateInfo::swapchainImageView cannot be VK_NULL_HANDLE!");
        }
        if (renderPass == nullptr) {
            throw std::runtime_error("IEFramebuffer::CreateInfo::renderPass cannot be a nullptr!");
        }
        clearValues[0].color = VkClearColorValue{defaultColor[0], defaultColor[1], defaultColor[2], defaultColor[3]};
        clearValues[1].depthStencil = {1.0f, 0};
        clearValues[2].color = clearValues[0].color;

        width = width == 0 ? static_cast<uint16_t>(linkedRenderEngine->swapchain.extent.width) : width;
        height = height == 0 ? static_cast<uint16_t>(linkedRenderEngine->swapchain.extent.height) : height;

        // Create framebuffer images
        IEImage::CreateInfo framebufferImageCreateInfo{
            .format=linkedRenderEngine->swapchain.image_format,
            .usage=VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            .allocationUsage=VMA_MEMORY_USAGE_GPU_ONLY
        };

        uint8_t msaaSamplesAllowed = getHighestMSAASampleCount(linkedRenderEngine->settings.msaaSamples);

        if (msaaSamplesAllowed > VK_SAMPLE_COUNT_1_BIT) {
            colorImage.create(linkedRenderEngine, &framebufferImageCreateInfo);
            deletionQueue.emplace_back([&] {
                colorImage.destroy();
            });
        }
        framebufferImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        framebufferImageCreateInfo.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
        framebufferImageCreateInfo.layout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        depthImage.create(linkedRenderEngine, &framebufferImageCreateInfo);
        deletionQueue.emplace_back([&] {
            depthImage.destroy();
        });
        std::vector<VkImageView> framebufferAttachments{msaaSamplesAllowed == VK_SAMPLE_COUNT_1_BIT ? swapchainImageView : colorImage.view, depthImage.view, swapchainImageView};
        VkFramebufferCreateInfo framebufferCreateInfo{
                .sType=VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
                .renderPass = renderPass,
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

    ~IEFramebuffer() override {
        destroy();
    }

private:
    std::vector<std::function<void()>> deletionQueue{};
};
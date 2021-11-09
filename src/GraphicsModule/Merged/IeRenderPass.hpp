#pragma once

#include "IeRenderEngineLink.hpp"
#include "IeFramebuffer.hpp"

class IeRenderPass {
public:
    struct CreateInfo {
    public:
        std::vector<std::pair<IeFramebuffer, std::vector<VkAttachmentDescription>>> attachments{};
        uint8_t msaaSamples{};
    };

    VkRenderPass renderPass{};
    CreateInfo createdWith{};
    IeRenderEngineLink *linkedRenderEngine{};

    void create(IeRenderEngineLink *engineLink, CreateInfo *createInfo) {
        linkedRenderEngine = engineLink;
        createdWith = *createInfo;
        for (auto attachment : createdWith.attachments) {

        }
        std::vector<VkAttachmentDescription> attachmentsDescriptions{};
        VkAttachmentReference colorAttachmentReference{};
        colorAttachmentReference.attachment = 0; // Index of attachment in array passed to VkRenderPassCreateInfo
        VkRenderPassCreateInfo renderPassCreateInfo{.attachmentCount = static_cast<uint32_t>(attachmentsDescriptions.size()), .pAttachments=attachmentsDescriptions.data(), .subpassCount=1};
        if (vkCreateRenderPass(linkedRenderEngine->device.device, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) {linkedRenderEngine->log->log("Failed to create render pass!", log4cplus::DEBUG_LOG_LEVEL, "Graphics Module"); }
    }
};

void IeFramebuffer::create(IeRenderEngineLink *engineLink, CreateInfo *createInfo, IeRenderPass *containerRenderPass) {
    createdWith = *createInfo;
    linkedRenderEngine = engineLink;
    createdWith.msaaSamples = std::min(createdWith.msaaSamples, linkedRenderEngine->settings.msaaSamples);
    IeImage::CreateInfo framebufferImageCreateInfo{.properties=IE_PRE_DESIGNED_COLOR_IMAGE, .width=linkedRenderEngine->swapchain.extent.width, .height=linkedRenderEngine->swapchain.extent.height, .msaaSamples=createdWith.msaaSamples};
    if ((createdWith.msaaSamples > 1) | (createdWith.aspects & IE_FRAMEBUFFER_ASPECT_COLOR_BIT)) {
        color.create(linkedRenderEngine, &framebufferImageCreateInfo);
        color.upload();
        created.colorImage = true;
    } if (createdWith.aspects & IE_FRAMEBUFFER_ASPECT_DEPTH_BIT) {
        framebufferImageCreateInfo.properties = IE_PRE_DESIGNED_DEPTH_IMAGE;
        depth.create(linkedRenderEngine, &framebufferImageCreateInfo);
        depth.upload();
        created.depthImage = true;
    }
    std::vector<VkImageView> framebufferAttachments{linkedRenderEngine->settings.msaaSamples <= 1 ? createdWith.swapchainImageView : color.view, depth.view, createdWith.swapchainImageView};
    VkFramebufferCreateInfo framebufferCreateInfo{.sType=VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO, .renderPass=containerRenderPass->renderPass};
    if (vkCreateFramebuffer(linkedRenderEngine->device.device, &framebufferCreateInfo, nullptr, &framebuffer) != VK_SUCCESS) { linkedRenderEngine->log->log("Failed to create framebuffer from images!", log4cplus::DEBUG_LOG_LEVEL, "Graphics Module"); }
}
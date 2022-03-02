#pragma once

#include "IEGraphicsLink.hpp"
#include "Image/IEFramebuffer.hpp"

#include <vulkan/vulkan.h>

class IERenderPass {
public:
    struct CreateInfo {
        std::vector<uint8_t> msaaSamples{1};
    } createdWith;

    VkRenderPass renderPass{};
    std::vector<IEFramebuffer> framebuffers{};

    void create(IEGraphicsLink *engineLink, CreateInfo *createInfo) {
        createdWith = *createInfo;
        linkedRenderEngine = engineLink;

        // Generate attachment descriptions
        uint8_t thisAttachmentMsaaSampleCount = std::min(linkedRenderEngine->settings.msaaSamples, createdWith.msaaSamples[0]);

        VkAttachmentDescription colorAttachmentDescription{
                .format=linkedRenderEngine->swapchain.image_format,
                .samples=static_cast<VkSampleCountFlagBits>(thisAttachmentMsaaSampleCount),
                .loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp=VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp=VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp=VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout=VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout=thisAttachmentMsaaSampleCount == VK_SAMPLE_COUNT_1_BIT ? VK_IMAGE_LAYOUT_PRESENT_SRC_KHR : VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };
        VkAttachmentDescription depthAttachmentDescription{
                .format=VK_FORMAT_D32_SFLOAT_S8_UINT,
                .samples=static_cast<VkSampleCountFlagBits>(thisAttachmentMsaaSampleCount),
                .loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp=VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp=VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp=VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout=VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout=VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
        };
        VkAttachmentDescription colorResolveAttachmentDescription{
                .format=linkedRenderEngine->swapchain.image_format,
                .samples=VK_SAMPLE_COUNT_1_BIT,
                .loadOp=VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp=VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp=VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp=VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout=VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout=VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
        };
        VkAttachmentReference colorAttachmentReference{
                .attachment=0,
                .layout=VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };
        VkAttachmentReference depthAttachmentReference{
                .attachment=1,
                .layout=VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        };
        VkAttachmentReference colorResolveAttachmentReference{
                .attachment=2,
                .layout=VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        };
        VkSubpassDescription subpassDescription{
                .pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS,
                .colorAttachmentCount=1,
                .pColorAttachments=&colorAttachmentReference,
                .pResolveAttachments=thisAttachmentMsaaSampleCount == VK_SAMPLE_COUNT_1_BIT ? nullptr : &colorResolveAttachmentReference,
                .pDepthStencilAttachment=&depthAttachmentReference
        };
        VkSubpassDependency subpassDependency{
                .srcSubpass=VK_SUBPASS_EXTERNAL,
                .dstSubpass=0,
                .srcStageMask=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                .dstStageMask=VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
                .srcAccessMask=0,
                .dstAccessMask=VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
        };
        std::vector<VkAttachmentDescription> attachmentDescriptions{
                colorAttachmentDescription,
                depthAttachmentDescription
        };
        if (thisAttachmentMsaaSampleCount > VK_SAMPLE_COUNT_1_BIT) {
            attachmentDescriptions.push_back(colorResolveAttachmentDescription);
        }
        VkRenderPassCreateInfo renderPassCreateInfo{
                .sType=VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .attachmentCount=static_cast<uint32_t>(attachmentDescriptions.size()),
                .pAttachments=attachmentDescriptions.data(),
                .subpassCount=1,
                .pSubpasses=&subpassDescription,
                .dependencyCount=1,
                .pDependencies=&subpassDependency
        };
        if (vkCreateRenderPass(linkedRenderEngine->device.device, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) { throw std::runtime_error("failed to create render pass!"); }
        deletionQueue.emplace_back([&] {
            vkDestroyRenderPass(linkedRenderEngine->device.device, renderPass, nullptr);
        });

        // Create framebuffers
        framebuffers.resize(createdWith.msaaSamples.size());
        IEFramebuffer::CreateInfo framebufferCreateInfo {
                .renderPass=renderPass,
        };
        for (uint32_t i = 0; i < createdWith.msaaSamples.size(); ++i) {
            framebufferCreateInfo.swapchainImageView = linkedRenderEngine->swapchainImageViews[i];
            framebuffers[i].create(linkedRenderEngine, &framebufferCreateInfo);
        }
    }

    VkRenderPassBeginInfo beginRenderPass(const IEFramebuffer &framebuffer) {
        VkRenderPassBeginInfo renderPassBeginInfo{
                .sType=VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
                .renderPass = renderPass,
                .framebuffer = framebuffer.framebuffer,
                .renderArea{
                        .offset = {0, 0},
                        .extent = linkedRenderEngine->swapchain.extent,
                },
                .clearValueCount = static_cast<uint32_t>(linkedRenderEngine->settings.msaaSamples == VK_SAMPLE_COUNT_1_BIT ? 2 : 3),
                .pClearValues = framebuffer.clearValues.data(),
        };
        return renderPassBeginInfo;
    }

    void destroy() {
        for (std::function<void()> &function : deletionQueue) {
            function();
        }
        deletionQueue.clear();
    }

    ~IERenderPass() {
        destroy();
    }

private:
    std::vector<std::function<void()>> deletionQueue{};
    IEGraphicsLink *linkedRenderEngine{};
};

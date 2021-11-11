#pragma once

#include <vector>

#include "IeImage.hpp"
#include "IeRenderEngineLink.hpp"
#include "IeRenderPass.hpp"

enum IeFramebufferAspect {
    IE_FRAMEBUFFER_ASPECT_DEPTH_BIT = 0b01,
    IE_FRAMEBUFFER_ASPECT_COLOR_BIT = 0b10,
    IE_FRAMEBUFFER_ASPECT_COLOR_ONLY = IE_FRAMEBUFFER_ASPECT_COLOR_BIT,
    IE_FRAMEBUFFER_ASPECT_DEPTH_ONLY = IE_FRAMEBUFFER_ASPECT_DEPTH_BIT,
    IE_FRAMEBUFFER_ASPECT_DEPTH_AND_COLOR = 0b11
};

enum IeFramebufferAttachmentFormat {
    IE_FRAMEBUFFER_DEPTH_ATTACHMENT_FORMAT = VK_FORMAT_D32_SFLOAT_S8_UINT
};

struct IeRenderPassAttachmentDescription {
    VkAttachmentDescription color;
    VkAttachmentDescription depth;
    VkAttachmentDescription resolve;
};

class IeRenderPass;

class IeFramebuffer {
public:
    struct CreateInfo {
        IeFramebufferAspect aspects{IE_FRAMEBUFFER_ASPECT_DEPTH_AND_COLOR};
        uint8_t msaaSamples{1};
        VkImageView swapchainImageView;
        IeImageFormat format;
        IeFramebuffer* dependentOn;
        IeFramebuffer* requiredBy;
        uint8_t subpass{0};
    };

    struct Created {
        bool colorImage{};
        bool depthImage{};
        bool framebuffer{};
    };

    CreateInfo createdWith{};
    Created created{};
    IeImage color{};
    IeImage depth{};
    std::vector<VkFramebuffer> framebuffers{};
    std::vector<VkClearValue> clearValues{3};
    IeRenderEngineLink *linkedRenderEngine{};



    void create(IeRenderEngineLink *engineLink, IeFramebuffer::CreateInfo *createInfo) {

    }

    void linkToRenderPass() {

    }

    /**
     * @brief Generates the necessary data to link this framebuffers to a renderpass.
     * @param createInfo
     */
    static IeRenderPassAttachmentDescription generateAttachmentDescriptions(IeFramebuffer::CreateInfo createInfo) {
        IeRenderPassAttachmentDescription renderPassAttachmentDescription{}; // prepare result data
        if (createInfo.aspects & IE_FRAMEBUFFER_ASPECT_COLOR_BIT) {
            auto format = ieImageFormats.find(createInfo.format);
            if (createInfo.msaaSamples <= 1) {
                renderPassAttachmentDescription.color = VkAttachmentDescription{
                        .format=format->second.first,
                        .samples=static_cast<VkSampleCountFlagBits>(createInfo.msaaSamples),
                        .loadOp=createInfo.dependentOn ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR,
                        .storeOp=createInfo.requiredBy ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE,
                        .stencilLoadOp=createInfo.dependentOn ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR,
                        .stencilStoreOp=createInfo.requiredBy ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE,
                        .initialLayout=VK_IMAGE_LAYOUT_UNDEFINED,
                        .finalLayout=createInfo.requiredBy ? VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
                };
            }
            else {
                renderPassAttachmentDescription.resolve = VkAttachmentDescription{
                        .format=format->second.first,
                        .samples=VK_SAMPLE_COUNT_1_BIT,
                        .loadOp=createInfo.dependentOn ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR,
                        .storeOp=createInfo.requiredBy ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE,
                        .stencilLoadOp=createInfo.dependentOn ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR,
                        .stencilStoreOp=createInfo.requiredBy ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE,
                        .initialLayout=VK_IMAGE_LAYOUT_UNDEFINED,
                        .finalLayout=VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
                };
            }
        }
        if (createInfo.aspects & IE_FRAMEBUFFER_ASPECT_DEPTH_BIT) {
            renderPassAttachmentDescription.depth = VkAttachmentDescription{
                    .format=static_cast<VkFormat>(IE_FRAMEBUFFER_DEPTH_ATTACHMENT_FORMAT),
                    .samples=static_cast<VkSampleCountFlagBits>(createInfo.msaaSamples),
                    .loadOp=createInfo.dependentOn ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp=createInfo.requiredBy ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .stencilLoadOp=createInfo.dependentOn ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .stencilStoreOp=createInfo.requiredBy ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout=VK_IMAGE_LAYOUT_UNDEFINED,
                    .finalLayout=VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            };
        }
        return renderPassAttachmentDescription;
    }
};
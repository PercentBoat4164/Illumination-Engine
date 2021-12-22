#pragma once

#include "IEImage.hpp"
#include "IERenderEngineLink.hpp"
#include "IERenderPass.hpp"
#include "IEFramebufferAttachment.hpp"

#ifdef ILLUMINATION_ENGINE_VULKAN
#include <vulkan/vulkan.hpp>
#endif

#include <vector>

enum IeFramebufferAttachmentFormat {
    #ifdef ILLUMINATION_ENGINE_VULKAN
    IE_FRAMEBUFFER_DEPTH_ATTACHMENT_FORMAT = VK_FORMAT_D32_SFLOAT_S8_UINT
    #else
    IE_FRAMEBUFFER_DEPTH_ATTACHMENT_FORMAT = GL_FLOAT_32_UNSIGNED_INT_24_8_REV
    #endif
};

struct IeRenderPassAttachmentDescription {
    #ifdef ILLUMINATION_ENGINE_VULKAN
    VkAttachmentDescription depth{};
    std::vector<VkAttachmentDescription> color{};
    std::vector<VkAttachmentDescription> resolve{};
    #endif
};

class IERenderPass;

class IEFramebuffer {
public:
    struct CreateInfo {
        IeFramebufferAspect aspects{IE_FRAMEBUFFER_ASPECT_DEPTH_AND_COLOR};
        uint8_t msaaSamples{1};
        #ifdef ILLUMINATION_ENGINE_VULKAN
        VkImageView swapchainImageView;
        #endif
        IeImageFormat format;
        IEFramebuffer* dependentOn;
        IEFramebuffer* requiredBy;
        uint32_t subpass{0};
        uint32_t colorImageCount{1};
    };

    struct Created {
        bool framebuffer{};
    };

    CreateInfo createdWith{};
    Created created{};
    IEFramebufferAttachment depth{};
    std::vector<IEFramebufferAttachment> colorAttachments{};
    std::vector<IEFramebufferAttachment> resolveAttachments{};
    #ifdef ILLUMINATION_ENGINE_VULKAN
    std::vector<VkFramebuffer> framebuffers{};
    std::vector<VkClearValue> clearValues{3};
    #endif
    IERenderEngineLink *linkedRenderEngine{};



    void create(IERenderEngineLink *engineLink, IEFramebuffer::CreateInfo *createInfo) {

    }

    void linkToRenderPass(IERenderPass* renderPass) {

    }

    /**
     * @brief Generates the necessary contentsString to link this framebuffers to a renderpass.
     * @param createInfo
     */
    static IeRenderPassAttachmentDescription generateAttachmentDescriptions(IERenderEngineLink* linkedRenderEngine, IEFramebuffer::CreateInfo* createInfo) {
        #ifdef ILLUMINATION_ENGINE_VULKAN
        IeRenderPassAttachmentDescription renderPassAttachmentDescription{}; // prepare result contentsString
        if (createInfo->aspects & IE_FRAMEBUFFER_ASPECT_COLOR_BIT) {
            if (createInfo->colorImageCount == 0) {
                IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Color bit set, but no color images requested! Creating a one color image.");
            }
            createInfo->colorImageCount = std::max(1u, createInfo->colorImageCount);
            auto format = ieImageFormats.find(createInfo->format);
            if (format == ieImageFormats.end()) {
                return renderPassAttachmentDescription;
            }
            renderPassAttachmentDescription.color = {
                    createInfo->colorImageCount,
                    VkAttachmentDescription{
                            .format=format->second.first,
                            /**@todo Fix this so that it is not always one.*/
                            .samples=VK_SAMPLE_COUNT_1_BIT,//static_cast<VkSampleCountFlagBits>(createInfo->msaaSamples),
                            .loadOp=createInfo->dependentOn ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR,
                            .storeOp=createInfo->requiredBy ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE,
                            .stencilLoadOp=createInfo->dependentOn ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR,
                            .stencilStoreOp=createInfo->requiredBy ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE,
                            .initialLayout=VK_IMAGE_LAYOUT_UNDEFINED,
                            .finalLayout=createInfo->requiredBy ? VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
                    }
            };
            if (createInfo->msaaSamples > 1) {
                renderPassAttachmentDescription.resolve = {
                        createInfo->colorImageCount,
                        VkAttachmentDescription{
                                .format=format->second.first,
                                .samples=VK_SAMPLE_COUNT_1_BIT,
                                .loadOp=createInfo->dependentOn ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR,
                                .storeOp=createInfo->requiredBy ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                .stencilLoadOp=createInfo->dependentOn ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR,
                                .stencilStoreOp=createInfo->requiredBy ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE,
                                .initialLayout=VK_IMAGE_LAYOUT_UNDEFINED,
                                .finalLayout=VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
                        }
                };
            }
        }
        if (createInfo->aspects & IE_FRAMEBUFFER_ASPECT_DEPTH_BIT) {
            renderPassAttachmentDescription.depth = VkAttachmentDescription{
                    .format=static_cast<VkFormat>(IE_FRAMEBUFFER_DEPTH_ATTACHMENT_FORMAT),
                    /**@todo Fix this so that it is not always one.*/
                    .samples=VK_SAMPLE_COUNT_1_BIT,//static_cast<VkSampleCountFlagBits>(createInfo->msaaSamples),
                    .loadOp=createInfo->dependentOn ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .storeOp=createInfo->requiredBy ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .stencilLoadOp=createInfo->dependentOn ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR,
                    .stencilStoreOp=createInfo->requiredBy ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE,
                    .initialLayout=VK_IMAGE_LAYOUT_UNDEFINED,
                    .finalLayout=VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            };
        }
        return renderPassAttachmentDescription;
        #else
        return IeRenderPassAttachmentDescription{};
        #endif
    }

    void destroy() {
        if (created.framebuffer) {
            #ifdef ILLUMINATION_ENGINE_VULKAN
            if (linkedRenderEngine->api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
                for (VkFramebuffer framebuffer: framebuffers) {
                    vkDestroyFramebuffer(linkedRenderEngine->device.device, framebuffer, nullptr);
                }
            }
            #endif
        }
    }

    ~IEFramebuffer() {
        destroy();
    }
};
#pragma once

#include "IeRenderEngineLink.hpp"
#include "IeFramebuffer.hpp"

#include <vector>

class IeRenderPass {
public:
    struct CreateInfo {
    public:
        std::vector<IeFramebuffer::CreateInfo> attachments;
    };

    VkRenderPass renderPass{};
    CreateInfo createdWith{};
    IeRenderEngineLink *linkedRenderEngine{};
    std::vector<VkSubpassDescription> subpasses{};

    void create(IeRenderEngineLink *engineLink, CreateInfo *createInfo) {
        linkedRenderEngine = engineLink;
        createdWith = *createInfo;
        uint8_t subpassCount = std::max_element(createdWith.attachments.begin(), createdWith.attachments.end(), findSubpassCount)->subpass;
        attachmentDescriptions.reserve(createdWith.attachments.size());
        for (IeFramebuffer::CreateInfo attachment : createdWith.attachments) {
            attachmentDescriptions.push_back(IeFramebuffer::generateAttachmentDescriptions(linkedRenderEngine, &attachment));
        }
        std::vector<SubpassData> subpassesData{subpassCount};
        uint32_t attachmentCount{};
        subpasses.resize(subpassCount);
        for (uint32_t i = 0; i < attachmentDescriptions.size(); ++i) {
            SubpassData subpassData{
                .depth={
                        .attachment=attachmentCount++,
                        .layout=attachmentDescriptions[i].depth.finalLayout
                }
            }; // Initialization of subpass data with values that work for the depth if it exists
            for (uint32_t j = 0; j < attachmentDescriptions[i].color.size(); ++j) {
                subpassData.color.push_back({
                        .attachment=attachmentCount++,
                        .layout=attachmentDescriptions[i].color[j].finalLayout
                });
                if (!attachmentDescriptions[i].resolve.empty()) {
                    subpassData.resolve.push_back({
                          .attachment=attachmentCount++,
                          .layout=attachmentDescriptions[i].resolve[j].finalLayout
                    });
                }
            }
            subpasses[createdWith.attachments[i].subpass - 1] = {
                .pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS,
                .colorAttachmentCount=static_cast<uint32_t>(subpassData.color.size()),
                .pColorAttachments=subpassData.color.data(),
                .pResolveAttachments=subpassData.resolve.data(),
                .pDepthStencilAttachment=&subpassData.depth
            };
        }
        VkRenderPassCreateInfo renderPassCreateInfo{
            .sType=VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .subpassCount=static_cast<uint32_t>(subpasses.size()),
            .pSubpasses=subpasses.data(),
        };
        if (vkCreateRenderPass(linkedRenderEngine->device.device, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) {
            linkedRenderEngine->log->log("Failed to create render pass!", log4cplus::ERROR_LOG_LEVEL, "Graphics Module");
        }
    }

private:
    std::vector<IeRenderPassAttachmentDescription> attachmentDescriptions;

    struct SubpassData {
        VkAttachmentReference depth{};
        std::vector<VkAttachmentReference> color{};
        std::vector<VkAttachmentReference> resolve{};
    };

    static bool findSubpassCount(IeFramebuffer::CreateInfo first, IeFramebuffer::CreateInfo second) {
        return first.subpass > second.subpass;
    }
};
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
        std::sort(createdWith.attachments.begin(), createdWith.attachments.end(),
                  [&](IeFramebuffer::CreateInfo first, IeFramebuffer::CreateInfo second) { return first.subpass > second.subpass; });
        attachmentDescriptions.clear();
        attachmentDescriptions.reserve(createdWith.attachments.size());
        for (IeFramebuffer::CreateInfo attachment : createdWith.attachments) { // @todo Eliminate need to sort.
            attachmentDescriptions.push_back(IeFramebuffer::generateAttachmentDescriptions(linkedRenderEngine, &attachment));
        }
        subpasses.clear();
        subpasses.reserve(createdWith.attachments[createdWith.attachments.size() - 1].subpass + 1);
        VkAttachmentReference depthAttachmentReference;
        std::vector<VkAttachmentReference> colorAttachmentReferences;
        std::vector<VkAttachmentReference> resolveAttachmentReferences;
        for (uint32_t i = 0; i < subpasses.capacity(); ++i) {
            for (uint32_t j = 0; j < attachmentDescriptions.size(); ++j) {
                if (createdWith.attachments[j].subpass == i) {
                    depthAttachmentReference = {
                            .attachment=j * 3 + 0,
                            .layout=attachmentDescriptions[j].depth.finalLayout // @todo Fix dpeth not being initialized.
                    };
                    if (!attachmentDescriptions[j].color.empty()) {
                        colorAttachmentReferences = {};
                        colorAttachmentReferences.reserve(attachmentDescriptions[j].color.size());

                        resolveAttachmentReferences = {};
                        resolveAttachmentReferences.reserve(attachmentDescriptions[j].resolve.size());
                    }
                    for (uint32_t k = 0; k < attachmentDescriptions[j].color.size(); ++k) {
                        colorAttachmentReferences.push_back({
                                .attachment=j * 3 + 1,
                                .layout=attachmentDescriptions[j].color[k].finalLayout
                        });
                        resolveAttachmentReferences.push_back({
                                .attachment=j * 3 + 2,
                                .layout=attachmentDescriptions[j].resolve[k].finalLayout
                        });
                    }
                }
            }
            subpasses.push_back(VkSubpassDescription{
                    .pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS,
                    .colorAttachmentCount=1,
                    .pColorAttachments=nullptr
            });
        }
//        VkRenderPassCreateInfo renderPassCreateInfo{};
//        if (vkCreateRenderPass(linkedRenderEngine->device.device, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) {
//            linkedRenderEngine->log->log("Failed to create render pass!", log4cplus::DEBUG_LOG_LEVEL, "Graphics Module");
//        }
    }

private:
    std::vector<IeRenderPassAttachmentDescription> attachmentDescriptions;
};
#pragma once

#include "IeRenderEngineLink.hpp"
#include "IeFramebuffer.hpp"

#include <vector>

class IeRenderPass {
public:
    struct CreateInfo {
    public:
        std::vector<IeFramebuffer::CreateInfo> framebufferCreateInfos{};
    };

    VkRenderPass renderPass{};
    CreateInfo createdWith{};
    IeRenderEngineLink *linkedRenderEngine{};

    void create(IeRenderEngineLink* engineLink, CreateInfo* createInfo) {
        linkedRenderEngine = engineLink;
        createdWith = *createInfo;
        uint32_t framebufferCount = createdWith.framebufferCreateInfos.size();
        uint32_t subpassCount = std::max_element(createdWith.framebufferCreateInfos.begin(), createdWith.framebufferCreateInfos.end(), attachmentSubpassIsGreater)->subpass;
        std::vector<VkSubpassDescription> subpassDescriptions{subpassCount};
        std::vector<VkSubpassDependency> subpassDependencies{subpassCount};
        IeRenderPassAttachmentDescription framebufferAttachmentDescription{framebufferCount};
        std::vector<VkAttachmentDescription> attachmentDescriptions{};
        std::vector<VkAttachmentReference> attachmentReferences{framebufferCount};
        std::vector<VkAttachmentReference*> depthAttachmentReferences{};
        std::vector<VkAttachmentReference*> colorAttachmentReferences{};
        std::vector<VkAttachmentReference*> resolveAttachmentReferences{};
        uint32_t attachmentCount{};
        for (uint32_t i = 0; i < framebufferCount; ++i) {
            framebufferAttachmentDescription = IeFramebuffer::generateAttachmentDescriptions(linkedRenderEngine, &createdWith.framebufferCreateInfos[i]);
            depthAttachmentReferences.push_back(&attachmentReferences[attachmentCount]);
            attachmentReferences[attachmentCount] = {
                    .attachment=attachmentCount++,
                    .layout=framebufferAttachmentDescription.depth.finalLayout
            };
            attachmentDescriptions.push_back(framebufferAttachmentDescription.depth);
            for (uint32_t j = 0; j < framebufferAttachmentDescription.color.size(); ++j) {
                colorAttachmentReferences.push_back(&attachmentReferences[attachmentCount]);
                attachmentReferences[attachmentCount] = {
                        .attachment=attachmentCount++,
                        .layout=framebufferAttachmentDescription.color[j].finalLayout
                };
                attachmentDescriptions.push_back(framebufferAttachmentDescription.color[j]);
                if (!framebufferAttachmentDescription.resolve.empty()) {
                    resolveAttachmentReferences.push_back(&attachmentReferences[attachmentCount]);
                    attachmentReferences[attachmentCount] = {
                            .attachment=attachmentCount++,
                            .layout=framebufferAttachmentDescription.resolve[j].finalLayout
                    };
                    attachmentDescriptions.push_back(framebufferAttachmentDescription.resolve[j]);
                }
            }
        }
        subpassDescriptions.push_back({
                .pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS,
                .colorAttachmentCount=static_cast<uint32_t>(colorAttachmentReferences.size()),
                .pColorAttachments=reinterpret_cast<const VkAttachmentReference *>(colorAttachmentReferences.data()),
                .pResolveAttachments=reinterpret_cast<const VkAttachmentReference *>(resolveAttachmentReferences.data()),
                .pDepthStencilAttachment=reinterpret_cast<const VkAttachmentReference *>(depthAttachmentReferences.data()),
        });
        VkRenderPassCreateInfo renderPassCreateInfo{
                .sType=VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
                .attachmentCount=attachmentCount,
                .pAttachments=attachmentDescriptions.data(),
                .subpassCount=subpassCount,
                .pSubpasses=subpassDescriptions.data()
        };
        vkCreateRenderPass(linkedRenderEngine->device.device, &renderPassCreateInfo, nullptr, &renderPass);
    }

    void destroy() const {
        vkDestroyRenderPass(linkedRenderEngine->device.device, renderPass, nullptr);
    }

    ~IeRenderPass() {
        destroy();
    }

private:
    struct SubpassData {
        VkAttachmentReference depth{};
        std::vector<VkAttachmentReference> color{};
        std::vector<VkAttachmentReference> resolve{};
        std::vector<VkAttachmentReference *> all{};

        void getAllAttachments() {
            all.clear();
            all.reserve(color.size() + resolve.size() + 1);
            all.push_back(&depth);
            for (uint32_t i = 0; i < color.size(); ++i) {
                all.push_back(&color[i]);
                all.push_back(&resolve[i]);
            }
        }
    };

    static bool attachmentSubpassIsGreater(IeFramebuffer::CreateInfo first, IeFramebuffer::CreateInfo second) {
        return first.subpass > second.subpass;
    }
};
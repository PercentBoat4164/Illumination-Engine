#pragma once

#include "IERenderEngineLink.hpp"
#include "IEFramebuffer.hpp"

#include <vector>

class IERenderPass {
public:
    struct CreateInfo {
    public:
        std::vector<IEFramebuffer::CreateInfo> framebufferCreateInfos{};
    };

    #ifdef ILLUMINATION_ENGINE_VULKAN
    VkRenderPass renderPass{};
    #endif
    CreateInfo createdWith{};
    IERenderEngineLink *linkedRenderEngine{};

    void create(IERenderEngineLink* engineLink, CreateInfo* createInfo) {
        #ifdef ILLUMINATION_ENGINE_VULKAN
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
            /**@todo: Create and use the framebuffer attachment class.*/
            framebufferAttachmentDescription = IEFramebuffer::generateAttachmentDescriptions(linkedRenderEngine, &createdWith.framebufferCreateInfos[i]);
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
        #endif
    }

    void destroy() const {
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (linkedRenderEngine->api.name == IE_RENDER_ENGINE_API_NAME_VULKAN) {
            vkDestroyRenderPass(linkedRenderEngine->device.device, renderPass, nullptr);
        }
        #endif
    }

    ~IERenderPass() {
        destroy();
    }

private:
    struct SubpassData {
        #ifdef ILLUMINATION_ENGINE_VULKAN
        VkAttachmentReference depth{};
        std::vector<VkAttachmentReference> color{};
        std::vector<VkAttachmentReference> resolve{};
        std::vector<VkAttachmentReference *> all{};
        #endif

        void getAllAttachments() {
            #ifdef ILLUMINATION_ENGINE_VULKAN
            all.clear();
            all.reserve(color.size() + resolve.size() + 1);
            all.push_back(&depth);
            for (uint32_t i = 0; i < color.size(); ++i) {
                all.push_back(&color[i]);
                all.push_back(&resolve[i]);
            }
            #endif
        }
    };

    static bool attachmentSubpassIsGreater(IEFramebuffer::CreateInfo first, IEFramebuffer::CreateInfo second) {
        return first.subpass > second.subpass;
    }
};
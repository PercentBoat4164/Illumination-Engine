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
        std::vector<IeRenderPassAttachmentDescription> attachmentDescriptions{framebufferCount};
        std::vector<VkAttachmentReference> attachmentReferences{framebufferCount};
        std::vector<VkAttachmentReference*> depthAttachmentReferences{};
        std::vector<VkAttachmentReference*> colorAttachmentReferences{};
        std::vector<VkAttachmentReference*> resolveAttachmentReferences{};
        uint32_t attachmentCount{};
        for (uint32_t i = 0; i < framebufferCount; ++i) {
            attachmentDescriptions[i] = IeFramebuffer::generateAttachmentDescriptions(linkedRenderEngine, &createdWith.framebufferCreateInfos[i]);
            depthAttachmentReferences.push_back(&attachmentReferences[attachmentCount]);
            attachmentReferences[attachmentCount] = {
                    .attachment=attachmentCount++,
                    .layout=attachmentDescriptions[i].depth.finalLayout
            };
            for (uint32_t j = 0; j < attachmentDescriptions[i].color.size(); ++j) {
                colorAttachmentReferences.push_back(&attachmentReferences[attachmentCount]);
                attachmentReferences[attachmentCount] = {
                        .attachment=attachmentCount++,
                        .layout=attachmentDescriptions[i].color[j].finalLayout
                };
                if (!attachmentDescriptions[i].resolve.empty()) {
                    resolveAttachmentReferences.push_back(&attachmentReferences[attachmentCount]);
                    attachmentReferences[attachmentCount] = {
                            .attachment=attachmentCount++,
                            .layout=attachmentDescriptions[i].resolve[j].finalLayout
                    };
                }
            }
        }
        VkSubpassDescription subpassDescription{
                .pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS,
                .colorAttachmentCount=static_cast<uint32_t>(colorAttachmentReferences.size()),
                .pColorAttachments=reinterpret_cast<const VkAttachmentReference *>(colorAttachmentReferences.data()),
                .pResolveAttachments=reinterpret_cast<const VkAttachmentReference *>(resolveAttachmentReferences.data()),
                .pDepthStencilAttachment=reinterpret_cast<const VkAttachmentReference *>(depthAttachmentReferences.data()),
        };
    }

//    void create(IeRenderEngineLink *engineLink, CreateInfo *createInfo) {
//        linkedRenderEngine = engineLink;
//        createdWith = *createInfo;
//        uint8_t subpassCount = std::max_element(createdWith.framebufferCreateInfos.begin(), createdWith.framebufferCreateInfos.end(), attachmentSubpassIsGreater)->subpass;
//        attachmentDescriptions.reserve(createdWith.framebufferCreateInfos.size());
//        for (IeFramebuffer::CreateInfo attachment : createdWith.framebufferCreateInfos) {
//            attachmentDescriptions.push_back(IeFramebuffer::generateAttachmentDescriptions(linkedRenderEngine, &attachment));
//        }
//        std::vector<SubpassData> subpassesData{subpassCount};
//        uint32_t attachmentCount{};
//        std::vector<VkSubpassDescription> subpassDescriptions{subpassCount};
//        std::vector<VkSubpassDependency> subpassDependencies{subpassCount};
//        for (uint32_t i = 0; i < attachmentDescriptions.size(); ++i) {
//            SubpassData subpassData{
//                .depth={
//                        .attachment=attachmentCount++,
//                        .layout=attachmentDescriptions[i].depth.finalLayout
//                }
//            }; // Initialization of subpass data with values that work for the depth if it exists
//            for (uint32_t j = 0; j < attachmentDescriptions[i].color.size(); ++j) {
//                subpassData.color.push_back({
//                        .attachment=attachmentCount++,
//                        .layout=attachmentDescriptions[i].color[j].finalLayout
//                });
//                if (!attachmentDescriptions[i].resolve.empty()) {
//                    subpassData.resolve.push_back({
//                          .attachment=attachmentCount++,
//                          .layout=attachmentDescriptions[i].resolve[j].finalLayout
//                    });
//                }
//            }
//            subpassesData[i] = subpassData;
//        }
//        for (uint32_t i = 0; i < subpassesData.size(); ++i) {
//            subpassDescriptions[createdWith.framebufferCreateInfos[i].subpass - 1] = {
//                    .pipelineBindPoint=VK_PIPELINE_BIND_POINT_GRAPHICS,
//                    .colorAttachmentCount=static_cast<uint32_t>(subpassesData[i].color.size()),
//                    .pColorAttachments=subpassesData[i].color.data(),
//                    .pResolveAttachments=subpassesData[i].resolve.data(),
//                    .pDepthStencilAttachment=&subpassesData[i].depth,
//            };
//            /**@todo Add mask bits for raytracing if necessary.*/
//            subpassDependencies[createdWith.framebufferCreateInfos[i].subpass] = {
//                    .srcSubpass = VK_SUBPASS_EXTERNAL,
//                    .dstSubpass = 0,
//                    .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
//                    .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
//                    .srcAccessMask = 0,
//                    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
//            };
//        }
//        std::vector<VkAttachmentReference *> allAttachments{};
//        for (SubpassData& i : subpassesData) {
//            i.getAllAttachments();
//            allAttachments.emplace(i.all.begin());
//        }
//        VkRenderPassCreateInfo renderPassCreateInfo{
//            .sType=VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
//            .attachmentCount=static_cast<uint32_t>(allAttachments.size()),
//            .pAttachments=reinterpret_cast<const VkAttachmentDescription *>(allAttachments.data()),
//            .subpassCount=static_cast<uint32_t>(subpassDescriptions.size()),
//            .pSubpasses=subpassDescriptions.data(),
//            .dependencyCount=static_cast<uint32_t>(subpassDependencies.size()),
//            .pDependencies=subpassDependencies.data()
//        };
//        if (vkCreateRenderPass(linkedRenderEngine->device.device, &renderPassCreateInfo, nullptr, &renderPass) != VK_SUCCESS) {
//            linkedRenderEngine->log->log("Failed to create render pass!", log4cplus::ERROR_LOG_LEVEL, "Graphics Module");
//        }
//    }

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
#pragma once

#include <vector>

#include "IeImage.hpp"
#include "IeRenderEngineLink.hpp"

class IeFramebuffer {
    struct CreateInfo {
        uint8_t aspects{0b11};
        uint8_t msaaSamples{1};
        VkImageView swapchainImageView{};
    };

    CreateInfo createdWith{};
    IeImage color{};
    IeImage depth{};
    VkFramebuffer framebuffer{};
    std::vector<VkClearValue> clearValues{3};
    IeRenderEngineLink *renderEngineLink{};

    void create(IeRenderEngineLink *engineLink, CreateInfo *createInfo) {
        createdWith = *createInfo;
        renderEngineLink = engineLink;
        createdWith.msaaSamples = std::min(createdWith.msaaSamples, renderEngineLink->settings.msaaSamples);
        IeImage::CreateInfo framebufferImageCreateInfo{.properties=COLOR, .width=renderEngineLink->swapchain.extent.width, .height=renderEngineLink->swapchain.extent.height, .msaaSamples=createdWith.msaaSamples};
        if (renderEngineLink->settings.msaaSamples > 1) {
            color.create(renderEngineLink, &framebufferImageCreateInfo);
            color.upload();
        }
        framebufferImageCreateInfo.properties = DEPTH;
        depth.create(renderEngineLink, &framebufferImageCreateInfo);
        depth.upload();
        std::vector<VkImageView> framebufferAttachments{renderEngineLink->settings.msaaSamples <= 1 ? createdWith.swapchainImageView : color.view, depth.view, createdWith.swapchainImageView};
    }
};
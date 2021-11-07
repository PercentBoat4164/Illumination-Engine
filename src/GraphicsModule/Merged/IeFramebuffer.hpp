#pragma once

#include <vector>

#include "IeImage.hpp"
#include "IeRenderEngineLink.hpp"

enum IeFramebufferAspect {
    IE_FRAMEBUFFER_ASPECT_DEPTH_BIT = 0b01,
    IE_FRAMEBUFFER_ASPECT_COLOR_BIT = 0b10,
    IE_FRAMEBUFFER_ASPECT_COLOR_ONLY = IE_FRAMEBUFFER_ASPECT_COLOR_BIT,
    IE_FRAMEBUFFER_ASPECT_DEPTH_ONLY = IE_FRAMEBUFFER_ASPECT_DEPTH_BIT,
    IE_FRAMEBUFFER_ASPECT_DEPTH_AND_COLOR = 0b11
};

class IeFramebuffer {
public:
    struct CreateInfo {
        IeFramebufferAspect aspects{IE_FRAMEBUFFER_ASPECT_DEPTH_AND_COLOR};
        uint8_t msaaSamples{1};
        VkImageView swapchainImageView{};
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
    VkFramebuffer framebuffer{};
    std::vector<VkClearValue> clearValues{3};
    IeRenderEngineLink *linkedRenderEngine{};
};
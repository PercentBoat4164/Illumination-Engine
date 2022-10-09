#pragma once

/* Predefine classes used with pointers or as return values for functions. */
class IERenderEngine;

/* Include classes used as attributes or function arguments. */
// Internal dependencies
#include "GraphicsModule/Image/IEImage.hpp"

// External dependencies
#include <vulkan/vulkan.h>

// System dependencies
#include <string>
#include <vector>

class IEFramebuffer : public IEDependency {
public:
    class Attachment {
    public:
        std::shared_ptr<IEImage> image{};
        VkImageLayout            forceLayout{};
        uint8_t                  location{};
    };

    struct CreateInfo {
        std::weak_ptr<IERenderPass> renderPass{};
        std::vector<float>          defaultColor{0.0F, 0.0F, 0.0F, 1.0F};
        std::vector<Attachment>     attachments{};
    };

    std::vector<VkFramebuffer>            framebuffers{};
    std::vector<VkClearValue>             clearValues{3};
    std::vector<float>                    defaultColor{1.0F, 0.0F, 0.0F, 1.0F};
    std::weak_ptr<IERenderPass>           renderPass{};
    std::vector<std::shared_ptr<IEImage>> attachments{};
    IERenderEngine                       *linkedRenderEngine{};
    uint16_t                              width{};
    uint16_t                              height{};

    VkFramebuffer getFramebuffer(uint8_t index);

    IEFramebuffer();

    ~IEFramebuffer() override;

    void create(IERenderEngine *engineLink, CreateInfo *createInfo);

    VkFramebuffer operator[](uint8_t index);
};
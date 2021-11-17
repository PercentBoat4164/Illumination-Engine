#include "IeImage.hpp"

enum IeFramebufferAspect {
    IE_FRAMEBUFFER_ASPECT_DEPTH_BIT = 0b01,
    IE_FRAMEBUFFER_ASPECT_COLOR_BIT = 0b10,
    IE_FRAMEBUFFER_ASPECT_COLOR_ONLY = IE_FRAMEBUFFER_ASPECT_COLOR_BIT,
    IE_FRAMEBUFFER_ASPECT_DEPTH_ONLY = IE_FRAMEBUFFER_ASPECT_DEPTH_BIT,
    IE_FRAMEBUFFER_ASPECT_DEPTH_AND_COLOR = 0b11
};

const static std::unordered_multimap<IeFramebufferAspect, std::pair<VkImageAspectFlagBits, uint32_t>> ieFramebufferAspects {
        {IE_FRAMEBUFFER_ASPECT_DEPTH_BIT,               {VK_IMAGE_ASPECT_DEPTH_BIT,              GL_DEPTH}},
        {IE_FRAMEBUFFER_ASPECT_COLOR_BIT,               {VK_IMAGE_ASPECT_COLOR_BIT,              GL_COLOR}},
        {IE_FRAMEBUFFER_ASPECT_DEPTH_ONLY,              {VK_IMAGE_ASPECT_DEPTH_BIT,              GL_DEPTH}},
        {IE_FRAMEBUFFER_ASPECT_COLOR_ONLY,              {VK_IMAGE_ASPECT_COLOR_BIT,              GL_COLOR}},
};

class IeFramebufferAttachment : public IeImage{
public:
    struct CreateInfo {
        uint32_t colorImageCount{};
        IeImageFormat format{};
        uint8_t msaaSamples{};
        IeFramebufferAttachment* dependentOn{};
        IeFramebufferAttachment* requiredBy{};
    };

    CreateInfo createdWith{};
    IeFramebufferAspect aspect{};
    VkAttachmentDescription description{};
    VkAttachmentReference reference{};

    void generateDescription() {
        if (!created.image) {
            return;
        }
        if (aspect & IE_FRAMEBUFFER_ASPECT_DEPTH_AND_COLOR) {
            linkedRenderEngine->log->log("Attempted to create attachment with depth and color aspects. A single attachment cannot have both depth and color!", log4cplus::WARN_LOG_LEVEL, "Graphics Module");
        }
        if (aspect & IE_FRAMEBUFFER_ASPECT_DEPTH_BIT) {
            if (createdWith.colorImageCount == 0) {
                linkedRenderEngine->log->log("Color bit set, but requested 0 color images. Creating one color image anyway.", log4cplus::WARN_LOG_LEVEL, "Graphics Module");
            }
            createdWith.colorImageCount = std::max(1u, createdWith.colorImageCount);
            auto format = ieImageFormats.find(createdWith.format);
            if (format != ieImageFormats.end()) {
                description = {
                        .format=format->second.first,
                        .samples=static_cast<VkSampleCountFlagBits>(createdWith.msaaSamples),
                        .loadOp=createdWith.dependentOn ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR,
                        .storeOp=createdWith.requiredBy ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE,
                        .stencilLoadOp=createdWith.dependentOn ? VK_ATTACHMENT_LOAD_OP_LOAD : VK_ATTACHMENT_LOAD_OP_CLEAR,
                        .stencilStoreOp=createdWith.requiredBy ? VK_ATTACHMENT_STORE_OP_STORE : VK_ATTACHMENT_STORE_OP_DONT_CARE,
                        .initialLayout=VK_IMAGE_LAYOUT_UNDEFINED,
                        .finalLayout=createdWith.requiredBy ? VK_IMAGE_LAYOUT_READ_ONLY_OPTIMAL_KHR : VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
                };
            }
        }
    }
};
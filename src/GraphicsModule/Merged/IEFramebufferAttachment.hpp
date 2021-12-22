#include "IEImage.hpp"

enum IeFramebufferAspect {
    IE_FRAMEBUFFER_ASPECT_DEPTH_BIT = 0b01,
    IE_FRAMEBUFFER_ASPECT_COLOR_BIT = 0b10,
    IE_FRAMEBUFFER_ASPECT_COLOR_ONLY = IE_FRAMEBUFFER_ASPECT_COLOR_BIT,
    IE_FRAMEBUFFER_ASPECT_DEPTH_ONLY = IE_FRAMEBUFFER_ASPECT_DEPTH_BIT,
    IE_FRAMEBUFFER_ASPECT_DEPTH_AND_COLOR = 0b11
};

#ifdef ILLUMINATION_ENGINE_VULKAN
const static std::unordered_multimap<IeFramebufferAspect, std::pair<VkImageAspectFlagBits, uint32_t>> ieFramebufferAspects {
        {IE_FRAMEBUFFER_ASPECT_DEPTH_BIT,               {VK_IMAGE_ASPECT_DEPTH_BIT,              GL_DEPTH}},
        {IE_FRAMEBUFFER_ASPECT_COLOR_BIT,               {VK_IMAGE_ASPECT_COLOR_BIT,              GL_COLOR}},
        {IE_FRAMEBUFFER_ASPECT_DEPTH_ONLY,              {VK_IMAGE_ASPECT_DEPTH_BIT,              GL_DEPTH}},
        {IE_FRAMEBUFFER_ASPECT_COLOR_ONLY,              {VK_IMAGE_ASPECT_COLOR_BIT,              GL_COLOR}},
};
#else
#ifdef ILLUMINATION_ENGINE_OPENGL
const static std::unordered_multimap<IeFramebufferAspect, std::pair<uint32_t, uint32_t>> ieFramebufferAspects {
        {IE_FRAMEBUFFER_ASPECT_DEPTH_BIT,               {GL_DEPTH,              GL_DEPTH}},
        {IE_FRAMEBUFFER_ASPECT_COLOR_BIT,               {GL_COLOR,              GL_COLOR}},
        {IE_FRAMEBUFFER_ASPECT_DEPTH_ONLY,              {GL_DEPTH,              GL_DEPTH}},
        {IE_FRAMEBUFFER_ASPECT_COLOR_ONLY,              {GL_COLOR,              GL_COLOR}},
};
#endif
#endif

class IEFramebufferAttachment : public IEImage{
public:
    struct CreateInfo {
        uint32_t colorImageCount{};
        IeImageFormat format{};
        uint8_t msaaSamples{};
        IEFramebufferAttachment* dependentOn{};
        IEFramebufferAttachment* requiredBy{};
    };

    CreateInfo createdWith{};
    IeFramebufferAspect aspect{};
    #ifdef ILLUMINATION_ENGINE_VULKAN
    VkAttachmentDescription description{};
    VkAttachmentReference reference{};
    #endif

    void generateDescription() {
        #ifdef ILLUMINATION_ENGINE_VULKAN
        if (!created.image) {
            return;
        }
        if (aspect & IE_FRAMEBUFFER_ASPECT_DEPTH_AND_COLOR) {
            IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Attempted to create attachment with depth and color aspects! Only one aspect is allowed.");
        }
        if (aspect & IE_FRAMEBUFFER_ASPECT_DEPTH_BIT) {
            if (createdWith.colorImageCount == 0) {
                IELogger::logDefault(ILLUMINATION_ENGINE_LOG_LEVEL_WARN, "Color bit set, but no color images requested! Creating a one color image.");
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
        #endif
    }
};
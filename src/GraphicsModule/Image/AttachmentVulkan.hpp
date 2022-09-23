#pragma once

#include "Attachment.hpp"
#include "ImageVulkan.hpp"

#include <vulkan/vulkan_core.h>

namespace IE::Graphics {
class RenderPass;
}  // namespace IE::Graphics

namespace IE::Graphics::detail {
class AttachmentVulkan : public IE::Graphics::Attachment, public IE::Graphics::detail::ImageVulkan {
public:
    static const std::unordered_map<IE::Graphics::Attachment::Preset, VkImageLayout> layoutFromPreset;

    template<typename... Args>
    explicit AttachmentVulkan(
      const std::weak_ptr<IE::Graphics::RenderEngine> &t_engineLink,
      IE::Graphics::Attachment::Preset                 t_preset = IE_ATTACHMENT_PRESET_COLOR_OUTPUT,
      Args... t_args
    );
};
}  // namespace IE::Graphics::detail
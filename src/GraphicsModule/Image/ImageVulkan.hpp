#pragma once

#include "Image.hpp"

#include <vulkan/vulkan_core.h>

namespace IE::Graphics::detail {
class ImageVulkan : public IE::Graphics::Image {
private:
    static const VkImageLayout        m_layouts[];
    static const VkFormatFeatureFlags m_features[];
    static const VkAccessFlags        m_accessFlags[];

public:
    virtual ~ImageVulkan() = default;

    VkImage       m_id{};
    VkImageLayout m_layout{};

    static VkImageLayout layoutFromPreset(Preset t_preset);

    static VkFormat formatFromPreset(Preset t_preset, IE::Graphics::RenderEngine *t_engineLink);

    static VkAccessFlags accessFlagsFromPreset(Preset t_preset);

    static VkFormatFeatureFlags featuresFromPreset(Preset t_preset);
};
}  // namespace IE::Graphics::detail
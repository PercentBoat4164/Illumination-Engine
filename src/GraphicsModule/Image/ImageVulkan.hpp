#pragma once

#include "Image.hpp"

#include <vulkan/vulkan_core.h>

namespace IE::Graphics::detail {
class ImageVulkan : public IE::Graphics::Image {
public:
    virtual ~ImageVulkan() = default;

    VkImage       m_id{};
    VkImageLayout m_layout{};

    static VkImageLayout layoutFromPreset(Preset t_type);
};
}  // namespace IE::Graphics::detail
#pragma once

#include "Image.hpp"

#include <vulkan/vulkan_core.h>

namespace IE::Graphics::detail {
class ImageVulkan : public IE::Graphics::Image {
public:
    VkImage m_id{};
    VkImageLayout m_layout{};
};
}  // namespace IE::Graphics::detail
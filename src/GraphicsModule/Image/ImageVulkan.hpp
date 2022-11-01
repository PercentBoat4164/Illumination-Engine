#pragma once

#include "Image.hpp"

#include <vulkan/vulkan_core.h>

namespace IE::Graphics::detail {
class ImageVulkan : public IE::Graphics::Image {
public:
    virtual ~ImageVulkan() = default;

    VkImage       m_id{};
    VkImageLayout m_layout{};

    static VkImageLayout layoutFromPreset(Preset t_preset);

    static VkFormat formatFromPreset(Preset t_preset);

    static VkPipelineStageFlags pipelineStageFromPreset(Preset t_preset);

    static VkAccessFlags accessFlagsFromPreset(Preset t_preset);
};
}  // namespace IE::Graphics::detail
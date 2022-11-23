#pragma once

#include "Image.hpp"

#include <vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

namespace IE::Graphics::detail {
class ImageVulkan : public IE::Graphics::Image {
private:
    static const VkImageLayout        m_layouts[];
    static const VkFormatFeatureFlags m_features[];
    static const VkPipelineStageFlags m_stages[];
    static const VkAccessFlags        m_accessFlags[];
    static const Intent               m_intents[];

public:
    virtual ~ImageVulkan() = default;

    VkImage       m_id{};
    VkImageLayout m_layout{};
    VmaAllocation m_allocation{};

    bool _createImage(Preset t_preset, uint64_t t_flags, IE::Core::MultiDimensionalVector<unsigned char> &t_data);

    static VkImageLayout layoutFromPreset(Preset t_preset);

    static VkFormat formatFromPreset(Preset t_preset, IE::Graphics::RenderEngine *t_engineLink);

    static VkPipelineStageFlags stageFromPreset(Preset t_preset);

    static VkAccessFlags accessFlagsFromPreset(Preset t_preset);

    static VkFormatFeatureFlags featuresFromPreset(Preset t_preset);

    static Intent intentFromPreset(Preset t_preset);
};
}  // namespace IE::Graphics::detail
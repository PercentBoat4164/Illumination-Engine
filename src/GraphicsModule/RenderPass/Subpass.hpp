#pragma once

#include <Image/Image.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace IE::Graphics {
class Subpass {
public:
    enum AttachmentConsumptionBits {
        IE_ATTACHMENT_CONSUMPTION_BITS_IGNORE  = 0x0,
        IE_ATTACHMENT_CONSUMPTION_BITS_INPUT   = 0x1,
        IE_ATTACHMENT_CONSUMPTION_BITS_CREATE  = 0x2,
        IE_ATTACHMENT_CONSUMPTION_BITS_OUTPUT  = 0x4,
        IE_ATTACHMENT_CONSUMPTION_BITS_DISCARD = 0x8,
    };

    using AttachmentConsumptionBit = uint8_t;

    enum AttachmentConsumptions {
        IE_ATTACHMENT_CONSUMPTION_IGNORE = IE_ATTACHMENT_CONSUMPTION_BITS_IGNORE,
        IE_ATTACHMENT_CONSUMPTION_CONSUME =
          IE_ATTACHMENT_CONSUMPTION_BITS_INPUT | IE_ATTACHMENT_CONSUMPTION_BITS_DISCARD,
        IE_ATTACHMENT_CONSUMPTION_MODIFY =
          IE_ATTACHMENT_CONSUMPTION_BITS_INPUT | IE_ATTACHMENT_CONSUMPTION_BITS_OUTPUT,
        IE_ATTACHMENT_CONSUMPTION_GENERATE =
          IE_ATTACHMENT_CONSUMPTION_BITS_CREATE | IE_ATTACHMENT_CONSUMPTION_BITS_OUTPUT,
        IE_ATTACHMENT_CONSUMPTION_TEMPORARY =
          IE_ATTACHMENT_CONSUMPTION_BITS_CREATE | IE_ATTACHMENT_CONSUMPTION_BITS_DISCARD,
    };

    using AttachmentConsumption = uint8_t;

    enum AttachmentUsages {
        IE_ATTACHMENT_USAGE_PRESERVE = 0x0,
        IE_ATTACHMENT_USAGE_INPUT    = 0x1,
        IE_ATTACHMENT_USAGE_COLOR    = 0x2,
        IE_ATTACHMENT_USAGE_RESOLVE  = 0x3,
        IE_ATTACHMENT_USAGE_DEPTH    = 0x4,
    };

    using AttachmentUsage = uint8_t;

    struct AttachmentDescription {
        AttachmentConsumption       m_consumption;
        AttachmentUsage             m_usage;
        IE::Graphics::Image::Preset m_type;
    } __attribute__((aligned(8)));

    enum Preset {
        IE_SUBPASS_PRESET_CUSTOM = 0x0,
    };

    explicit Subpass(Preset t_preset, VkPipelineStageFlags t_stage = 0x0);

    Preset                                                     m_preset;
    std::vector<std::pair<std::string, AttachmentDescription>> m_attachments{};
    VkPipelineStageFlags                                       m_stage{};
    std::vector<VkAttachmentReference>                         m_input;
    std::vector<VkAttachmentReference>                         m_color;
    std::vector<VkAttachmentReference>                         m_resolve;
    std::vector<VkAttachmentReference>                         m_depth;
    std::vector<uint32_t>                                      m_preserve;

    auto addOrModifyAttachment(
      const std::string                           &t_attachmentName,
      IE::Graphics::Subpass::AttachmentConsumption t_consumption,
      IE::Graphics::Subpass::AttachmentUsage       t_usage,
      Image::Preset                                t_type
    ) -> decltype(*this);

    static VkPipelineStageFlags stageFromPreset(Preset t_preset);
};
}  // namespace IE::Graphics
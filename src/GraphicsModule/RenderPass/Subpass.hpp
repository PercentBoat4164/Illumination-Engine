#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace IE::Graphics {
class Subpass {
public:
    enum AttachmentUsages {
        IE_ATTACHMENT_USAGE_IGNORE       = 0x0,
        IE_ATTACHMENT_USAGE_CONSUME      = 0x1,
        IE_ATTACHMENT_USAGE_MODIFY       = 0x2,
        IE_ATTACHMENT_USAGE_GENERATE     = 0x3,
        IE_ATTACHMENT_USAGE_TEMPORARY = 0x4,
    };

    using AttachmentUsage = uint64_t;

    enum Preset {
        IE_SUBPASS_PRESET_CUSTOM = 0x0,
    };

    explicit Subpass(Preset t_preset);

    Preset                                           m_preset;
    std::unordered_map<std::string, AttachmentUsage> m_attachments{};

    auto addOrModifyAttachment(const std::string &t_attachmentName, AttachmentUsage t_influence)
      -> decltype(*this);
};
}  // namespace IE::Graphics
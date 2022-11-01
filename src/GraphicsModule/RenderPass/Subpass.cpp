#include "Subpass.hpp"

#include "Image/Image.hpp"

const VkPipelineStageFlags IE::Graphics::Subpass::m_stages[]{VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT};

auto IE::Graphics::Subpass::addOrModifyAttachment(
  const std::string                           &t_attachmentName,
  IE::Graphics::Subpass::AttachmentConsumption t_consumption,
  IE::Graphics::Subpass::AttachmentUsage       t_usage,
  Image::Preset                                t_type
) -> decltype(*this) {
    auto iterator = std::find_if(
      m_attachments.begin(),
      m_attachments.end(),
      [&](std::pair<std::string, AttachmentDescription> &t) { return t.first == t_attachmentName; }
    );
    if (iterator == m_attachments.end()) {
        m_attachments.push_back({
          t_attachmentName,
          {t_consumption, t_usage, t_type}
        });
    } else iterator->second = {t_consumption, t_usage, t_type};
    return *this;
}

IE::Graphics::Subpass::Subpass(IE::Graphics::Subpass::Preset t_preset, VkPipelineStageFlags t_stage) :
        m_preset(t_preset),
        m_stage(t_preset == IE_SUBPASS_PRESET_CUSTOM ? t_stage : stageFromPreset(t_preset)) {
}

VkPipelineStageFlags IE::Graphics::Subpass::stageFromPreset(IE::Graphics::Subpass::Preset t_preset) {
    return m_stages[t_preset];
}

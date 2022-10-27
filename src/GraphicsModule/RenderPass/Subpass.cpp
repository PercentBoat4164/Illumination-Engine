#include "Subpass.hpp"

auto IE::Graphics::Subpass::addOrModifyAttachment(
  const std::string                         &t_attachmentName,
  IE::Graphics::Subpass::AttachmentUsage t_influence
) -> decltype(*this) {
    m_attachments[t_attachmentName] = m_attachments[t_attachmentName] ^ t_influence;
    return *this;
}

IE::Graphics::Subpass::Subpass(IE::Graphics::Subpass::Preset t_preset) : m_preset(t_preset) {
}

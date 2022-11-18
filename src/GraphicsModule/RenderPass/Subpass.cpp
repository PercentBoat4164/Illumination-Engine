#include "Subpass.hpp"

#include "Image/Image.hpp"
#include "RenderPass.hpp"

IE::Graphics::Subpass::Subpass(
  IE::Graphics::Subpass::Preset                       t_preset,
  std::vector<std::shared_ptr<IE::Graphics::Shader>> &t_shaders
) :
        m_preset(t_preset),
        shaders(t_shaders) {
    for (auto &shader : shaders) shader->m_subpass = this;
    pipeline.m_subpass = this;
}

auto IE::Graphics::Subpass::addOrModifyAttachment(
  const std::string                           &t_attachmentName,
  IE::Graphics::Subpass::AttachmentConsumption t_consumption,
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
          {t_consumption, t_type}
        });
    } else iterator->second = {t_consumption, t_type};
    return *this;
}

void IE::Graphics::Subpass::build(IE::Graphics::RenderPass *t_renderPass) {
    m_renderPass = t_renderPass;
    for (auto &shader : shaders) {
        shader->build(this);
        shader->compile();
    }
    descriptorSet.build(this, shaders);
    pipeline.build(this, shaders);
}

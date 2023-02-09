#include "Subpass.hpp"

#include "Image/Image.hpp"
#include "Renderable/Renderable.hpp"
#include "RenderEngine.hpp"
#include "RenderPass.hpp"

IE::Graphics::Subpass::Subpass(
  IE::Graphics::Subpass::Preset                       t_preset,
  std::vector<std::shared_ptr<IE::Graphics::Shader>> &t_shaders
) :
        m_preset(t_preset),
        shaders(t_shaders) {
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
    m_linkedRenderEngine = t_renderPass->m_linkedRenderEngine;
    m_renderPass         = t_renderPass;

    for (auto &shader : shaders) {
        shader->build(m_linkedRenderEngine);
        shader->compile();
    }
    descriptorSet.build(this, shaders);
    pipeline.build(this, shaders);
}

void IE::Graphics::Subpass::registerRenderable(IE::Graphics::Renderable *t_renderable) {
    if (t_renderable->m_linkedRenderEngine != m_linkedRenderEngine) {
        m_linkedRenderEngine->getLogger().log(
          "Attempted to link a foreign renderable to this render engine!",
          Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
        t_renderable->m_linkedRenderEngine->getLogger().log(
          "Attempted to link this renderable to a foreign render engine!",
          Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    }
    auto buffer{std::make_shared<CommandBuffer>(m_linkedRenderEngine->getCommandPool())};

    buffer->allocate(IE_COMMAND_BUFFER_ALLOCATE_SECONDARY);
    buffer->record(0, m_renderPass, 0, &m_renderPass->m_framebuffer);
}

void IE::Graphics::Subpass::execute(IE::Graphics::CommandBuffer t_masterCommandBuffer) {
    t_masterCommandBuffer.recordExecuteSecondaryCommandBuffers(m_commandBuffers);
}

void IE::Graphics::Subpass::destroy() {
    descriptorSet.destroy();
    pipeline.destroy();
    for (auto &shader : shaders) shader->destroy();
}

IE::Graphics::Subpass::~Subpass() {
    destroy();
}

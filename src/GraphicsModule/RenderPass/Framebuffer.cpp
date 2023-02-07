#include "Framebuffer.hpp"

#include "GraphicsModule/Image/ImageVulkan.hpp"
#include "GraphicsModule/RenderEngine.hpp"

void IE::Graphics::Framebuffer::build(
  IE::Graphics::RenderPass                 *t_renderPass,
  std::vector<IE::Graphics::Image::Preset> &t_attachmentPresets
) {
    m_renderPass    = t_renderPass;
    m_resolution[0] = m_renderPass->m_renderPassSeries->m_linkedRenderEngine->m_currentResolution[0];
    m_resolution[1] = m_renderPass->m_renderPassSeries->m_linkedRenderEngine->m_currentResolution[1];

    IE::Core::MultiDimensionalVector<unsigned char> tmp{};
    attachments.reserve(t_attachmentPresets.size());
    for (size_t i{}; i < t_attachmentPresets.size(); ++i) {
        attachments.push_back(std::make_shared<IE::Graphics::detail::ImageVulkan>(
          m_renderPass->m_renderPassSeries->m_linkedRenderEngine
        ));
        attachments[i]->createImage(t_attachmentPresets[i], 0x0, tmp);
    }

    // Get attachment views
    std::vector<VkImageView> attachmentViews;
    attachmentViews.reserve(attachments.size());
    for (std::shared_ptr<IE::Graphics::Image> &attachment : attachments)
        attachmentViews.push_back(
          std::reinterpret_pointer_cast<IE::Graphics::detail::ImageVulkan>(attachment)->m_view
        );

    // Create framebuffer
    VkFramebufferCreateInfo framebufferCreateInfo{
      .sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .pNext           = nullptr,
      .flags           = 0x0,
      .renderPass      = m_renderPass->m_renderPass,
      .attachmentCount = static_cast<uint32_t>(attachmentViews.size()),
      .pAttachments    = attachmentViews.data(),
      .width           = static_cast<uint32_t>(m_resolution[0]),
      .height          = static_cast<uint32_t>(m_resolution[1]),
      .layers          = 0x1};
    VkResult result{vkCreateFramebuffer(
      m_renderPass->m_renderPassSeries->m_linkedRenderEngine->m_device.device,
      &framebufferCreateInfo,
      nullptr,
      &m_framebuffer
    )};
    if (result != VK_SUCCESS)
        m_renderPass->m_renderPassSeries->m_linkedRenderEngine->getLogger().log(
          "Failed to create framebuffer! Error: " + IE::Graphics::RenderEngine::translateVkResultCodes(result),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    else m_renderPass->m_renderPassSeries->m_linkedRenderEngine->getLogger().log("Created Framebuffer");
}

IE::Graphics::Framebuffer::~Framebuffer() {
    destroy();
}

void IE::Graphics::Framebuffer::destroy() {
    for (auto &attachment : attachments) attachment->destroyImage();
    vkDestroyFramebuffer(
      m_renderPass->m_renderPassSeries->m_linkedRenderEngine->m_device.device,
      m_framebuffer,
      nullptr
    );
}

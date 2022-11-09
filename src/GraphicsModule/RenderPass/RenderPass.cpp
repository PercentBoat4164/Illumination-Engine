#include "RenderPass.hpp"

#include "RenderEngine.hpp"

IE::Graphics::RenderPass::RenderPass(Preset t_preset) : m_preset(t_preset) {
}

void IE::Graphics::RenderPass::build(
  RenderPassSeries                     *t_renderPassSeries,
  std::vector<VkAttachmentDescription> &t_attachmentDescriptions,
  std::vector<VkSubpassDescription>    &t_subpassDescriptions,
  std::vector<VkSubpassDependency>     &t_subpassDependency
) {
    m_renderPassSeries = t_renderPassSeries;

    // Build this render pass
    VkRenderPassCreateInfo renderPassCreateInfo{
      .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .pNext           = nullptr,
      .flags           = 0x0,
      .attachmentCount = static_cast<uint32_t>(t_attachmentDescriptions.size()),
      .pAttachments    = t_attachmentDescriptions.data(),
      .subpassCount    = static_cast<uint32_t>(t_subpassDescriptions.size()),
      .pSubpasses      = t_subpassDescriptions.data(),
      .dependencyCount = static_cast<uint32_t>(t_subpassDependency.size()),
      .pDependencies   = t_subpassDependency.data()};
    VkResult result{vkCreateRenderPass(
      m_renderPassSeries->m_linkedRenderEngine->m_device.device,
      &renderPassCreateInfo,
      nullptr,
      &m_renderPass
    )};
    if (result != VK_SUCCESS)
        m_renderPassSeries->m_linkedRenderEngine->getLogger().log(
          "Failed to create Render Pass with error: " + RenderEngine::translateVkResultCodes(result),
          IE::Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    else m_renderPassSeries->m_linkedRenderEngine->getLogger().log("Created Render Pass");

    // Build all the subpasses controlled by this render pass.
    for (auto subpass : m_subpasses) subpass.build(this);
}

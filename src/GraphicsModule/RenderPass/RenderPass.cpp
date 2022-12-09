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
    for (auto &subpass : m_subpasses) subpass.build(this);
}

bool IE::Graphics::RenderPass::start(
  std::shared_ptr<CommandBuffer> masterCommandBuffer,
  IE::Graphics::Framebuffer     &framebuffer
) {
    m_currentPass = 0;
    VkClearValue              defaultClearValue{0, 0, 0};
    std::vector<VkClearValue> clearColors(framebuffer.attachments.size(), defaultClearValue);
    VkRenderPassBeginInfo     renderPassBeginInfo{
          .sType       = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
          .pNext       = nullptr,
          .renderPass  = m_renderPass,
          .framebuffer = framebuffer.m_framebuffer,
          .renderArea =
        VkRect2D{
                 .offset = VkOffset2D{.x = 0, .y = 0},
                 .extent =
            VkExtent2D{
                  .width  = static_cast<uint32_t>(framebuffer.m_resolution[0]),
                  .height = static_cast<uint32_t>(framebuffer.m_resolution[1]),
            }},
          .clearValueCount = static_cast<uint32_t>(framebuffer.attachments.size())
    };
    masterCommandBuffer->recordBeginRenderPass(
      &renderPassBeginInfo,
      VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS
    );
    return true;
}

bool IE::Graphics::RenderPass::nextPass(std::shared_ptr<IE::Graphics::CommandBuffer> masterCommandBuffer) {
    if (++m_currentPass < m_subpasses.size()) {
        masterCommandBuffer->recordNextSubpass();
        return IE_RENDER_PASS_PROGRESSION_STATUS_NEXT_SUBPASS_IN_SAME_RENDER_PASS;
    }
    return IE_RENDER_PASS_PROGRESSION_STATUS_NEXT_RENDER_PASS;
}

void IE::Graphics::RenderPass::finish(std::shared_ptr<CommandBuffer> masterCommandBuffer) {
    masterCommandBuffer->recordEndRenderPass();
}

#include "RenderPass.hpp"

#include "Image/AttachmentVulkan.hpp"
#include "RenderEngine.hpp"

#include <iostream>
#include <memory>
#include <utility>

template<typename T>
constexpr std::vector<T> &operator<<(std::vector<T> &t_first, const std::vector<T> &t_second) {
    size_t i{t_first.size()};
    size_t j{};
    t_first.reserve(t_first.size() + t_second.size());
    for (; i < t_first.capacity(); ++i) t_first.push_back(t_second[j++]);
    return t_first;
}

auto IE::Graphics::RenderPass::addSubpass(const IE::Graphics::Subpass &t_subpass) -> decltype(*this) {
    m_subpasses.push_back(t_subpass);
    return *this;
}

auto IE::Graphics::RenderPass::setResolution(std::array<size_t, 2> t_resolution) -> decltype(*this) {
    m_resolution = t_resolution;
    return *this;
}

auto IE::Graphics::RenderPass::setResolution(size_t t_width, size_t t_height) -> decltype(*this) {
    m_resolution = {t_width, t_height};
    return *this;
}

auto IE::Graphics::RenderPass::addSubpass(const std::vector<IE::Graphics::Subpass> &t_subpass) -> decltype(*this) {
    m_subpasses.reserve(m_subpasses.size() + t_subpass.size());
    for (const IE::Graphics::Subpass &subpass : t_subpass) m_subpasses.push_back(subpass);
    return *this;
}

auto IE::Graphics::RenderPass::addOutput(const std::string &t_attachment) -> decltype(*this) {
    m_outputAttachments.push_back(t_attachment);
    return *this;
}

auto IE::Graphics::RenderPass::addOutput(const std::vector<std::string> &t_attachments) -> decltype(*this) {
    m_outputAttachments.reserve(m_outputAttachments.size() + t_attachments.size());
    for (const std::string &attachment : t_attachments) m_outputAttachments.push_back(attachment);
    return *this;
}

std::vector<std::weak_ptr<IE::Graphics::Attachment>> IE::Graphics::RenderPass::build() {
    // Verify integrity of subpasses and create attachments
    for (const auto &subpass : m_subpasses) {
        if (!subpass.m_resolveAttachments.empty() &&
        subpass.m_colorAttachments.size() !=
            subpass.m_resolveAttachments.size()) {
            std::cout << "The number of color attachments must match the number of "
                         "resolve attachments.\n";
        }
        for (const auto &attachment : subpass.m_inputAttachments) {
            std::shared_ptr<IE::Graphics::Attachment> &output = m_attachments[attachment];
            if (!output)
                output = std::make_shared<IE::Graphics::detail::AttachmentVulkan>(
                  m_linkedRenderEngine,
                  subpass.attachmentPresets.at(attachment)
                );
        }
        for (const auto &attachment : subpass.m_colorAttachments) {
            std::shared_ptr<IE::Graphics::Attachment> &output = m_attachments[attachment];
            if (!output)
                output = std::make_shared<IE::Graphics::detail::AttachmentVulkan>(
                  m_linkedRenderEngine,
                  subpass.attachmentPresets.at(attachment)
                );
        }
        for (const auto &attachment : subpass.m_resolveAttachments) {
            std::shared_ptr<IE::Graphics::Attachment> &output = m_attachments[attachment];
            if (!output)
                output = std::make_shared<IE::Graphics::detail::AttachmentVulkan>(
                  m_linkedRenderEngine,
                  subpass.attachmentPresets.at(attachment)
                );
        }
        std::shared_ptr<IE::Graphics::Attachment> &output = m_attachments[subpass.m_depthStencilAttachment];
        if (!output)
            output = std::make_shared<IE::Graphics::detail::AttachmentVulkan>(
              m_linkedRenderEngine,
              subpass.attachmentPresets.at(subpass.m_depthStencilAttachment)
            );
    }

    m_attachmentStateHistory.resize(m_attachments.size());
    m_subpassDescriptions.reserve(m_subpasses.size());

    // Generate subpasses
    for (size_t i{}; i < m_subpasses.size(); ++i) {
        // Handle input attachments
        std::vector<VkAttachmentReference> inputAttachments{
          buildReferencesFromSource(m_subpasses[i].m_inputAttachments)};

        // Handle color attachments
        std::vector<VkAttachmentReference> colorAttachments{
          buildReferencesFromSource(m_subpasses[i].m_colorAttachments)};

        // Handle resolve attachments
        std::vector<VkAttachmentReference> resolveAttachments{
          buildReferencesFromSource(m_subpasses[i].m_resolveAttachments)};

        // Handle depth/stencil attachment
        VkAttachmentReference *depthStencilAttachment{
          m_subpasses[i].m_depthStencilAttachment.empty() ?
            nullptr :
            buildAttachmentReference(m_subpasses[i].m_depthStencilAttachment)};

        // Handle preserved attachments
        std::vector<uint32_t> preserveAttachments;

        // Get preserved attachments: attachments that are not used by this pass,
        // but are used by one or more future subpasses, or are outputs of the
        // render pass.
        for (const auto &attachment : m_attachments) {
            // Search for attachment in all subsequent subpasses
            if (findNextAttachmentUseAfter(i, attachment.first)) {
                // If the attachment is found, it needs to be preserved. The index of
                // the preserved attachment is recorded.
                preserveAttachments.push_back(
                  std::distance(m_attachments.begin(), m_attachments.find(attachment.first))
                );
            }
        }

        // Generate final subpass description
        m_subpassDescriptions.push_back(
          {.flags                   = 0x0,
           .pipelineBindPoint       = m_subpasses[i].m_bindPoint,
           .inputAttachmentCount    = static_cast<uint32_t>(inputAttachments.size()),
           .pInputAttachments       = inputAttachments.data(),
           .colorAttachmentCount    = static_cast<uint32_t>(colorAttachments.size()),
           .pColorAttachments       = colorAttachments.data(),
           .pResolveAttachments     = resolveAttachments.data(),
           .pDepthStencilAttachment = depthStencilAttachment,
           .preserveAttachmentCount = static_cast<uint32_t>(preserveAttachments.size()),
           .pPreserveAttachments    = preserveAttachments.data()}
        );
    }

    // Return output images for general use
    std::vector<std::weak_ptr<IE::Graphics::Attachment>> attachments(m_outputAttachments.size());
    size_t                                               i{};
    std::generate(attachments.begin(), attachments.end(), [&] {
        return std::weak_ptr(m_attachments[m_outputAttachments[i]]);
    });

    std::vector<VkAttachmentDescription> allDescriptions(m_attachments.size());
    i = 0;
    for (const auto &attachment : m_attachments) buildAttachmentDescriptionForSubpass(i, attachment.first);

    std::vector<VkSubpassDependency> allDependencies{};


    VkRenderPassCreateInfo renderPassCreateInfo{
      .sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = static_cast<uint32_t>(allDescriptions.size()),
      .pAttachments    = allDescriptions.data(),
      .subpassCount    = static_cast<uint32_t>(m_subpassDescriptions.size()),
      .pSubpasses      = m_subpassDescriptions.data(),
      .dependencyCount = static_cast<uint32_t>(allDependencies.size()),
      .pDependencies   = allDependencies.data()};

    vkCreateRenderPass(
      m_linkedRenderEngine.lock()->m_device.device,
      &renderPassCreateInfo,
      nullptr,
      &m_renderPass
    );

    return attachments;
}

auto IE::Graphics::RenderPass::addSubpass() -> decltype(*this) {
    m_subpasses.emplace_back();
    return *this;
}

auto IE::Graphics::RenderPass::takesInput(
  const std::string               &t_attachment,
  IE::Graphics::Attachment::Preset t_preset
) -> decltype(*this) {
    m_subpasses[m_subpasses.size() - 1].takesInput(t_attachment);
    return *this;
}

auto IE::Graphics::RenderPass::takesInput(
  const std::vector<std::string>  &t_attachments,
  IE::Graphics::Attachment::Preset t_preset
) -> decltype(*this) {
    m_subpasses[m_subpasses.size() - 1].takesInput(t_attachments);
    return *this;
}

auto IE::Graphics::RenderPass::require(const std::string &t_attachment, IE::Graphics::Attachment::Preset t_preset)
  -> decltype(*this) {
    m_subpasses[m_subpasses.size() - 1].require(t_attachment);
    return *this;
}

auto IE::Graphics::RenderPass::require(
  const std::vector<std::string>  &t_attachments,
  IE::Graphics::Attachment::Preset t_preset
) -> decltype(*this) {
    m_subpasses[m_subpasses.size() - 1].require(t_attachments);
    return *this;
}

auto IE::Graphics::RenderPass::recordsColorTo(
  const std::string               &t_attachment,
  IE::Graphics::Attachment::Preset t_preset
) -> decltype(*this) {
    m_subpasses[m_subpasses.size() - 1].recordsColorTo(t_attachment);
    return *this;
}

auto IE::Graphics::RenderPass::recordsColorTo(
  const std::vector<std::string>  &t_attachments,
  IE::Graphics::Attachment::Preset t_preset
) -> decltype(*this) {
    m_subpasses[m_subpasses.size() - 1].recordsColorTo(t_attachments);
    return *this;
}

auto IE::Graphics::RenderPass::resolvesTo(
  const std::string               &t_attachment,
  IE::Graphics::Attachment::Preset t_preset
) -> decltype(*this) {
    m_subpasses[m_subpasses.size() - 1].resolvesTo(t_attachment);
    return *this;
}

auto IE::Graphics::RenderPass::resolvesTo(
  const std::vector<std::string>  &t_attachments,
  IE::Graphics::Attachment::Preset t_preset
) -> decltype(*this) {
    m_subpasses[m_subpasses.size() - 1].resolvesTo(t_attachments);
    return *this;
}

auto IE::Graphics::RenderPass::recordsDepthStencilTo(
  const std::string               &t_attachment,
  IE::Graphics::Attachment::Preset t_preset
) -> RenderPass {
    m_subpasses[m_subpasses.size() - 1].recordsDepthStencilTo(
      t_attachment,
      Attachment::IE_ATTACHMENT_PRESET_SHADOW_MAP_MULTISAMPLE
    );
    return *this;
}

auto IE::Graphics::RenderPass::setBindPoint(VkPipelineBindPoint t_bindPoint) -> decltype(*this) {
    m_subpasses[m_subpasses.size() - 1].setBindPoint(t_bindPoint);
    return *this;
}

std::vector<std::string> IE::Graphics::RenderPass::getAllAttachmentsInSubpass(const IE::Graphics::Subpass &subpass
) {
    std::vector<std::string> allAttachments;
    (allAttachments << subpass.m_inputAttachments << subpass.m_colorAttachments << subpass.m_resolveAttachments)
      .push_back(subpass.m_depthStencilAttachment);
    return allAttachments;
}

IE::Graphics::Subpass *IE::Graphics::RenderPass::findNextAttachmentUseAfter(size_t i, const std::string &t_att) {
    for (; i < m_subpasses.size(); ++i) {
        std::vector<std::string> attachments{getAllAttachmentsInSubpass(m_subpasses[i])};
        if (std::find_if(attachments.begin(), attachments.end(), [&](const std::string &str) {
                return str == t_att;
            }) != attachments.end())
            return &m_subpasses[i];
    }
    return nullptr;
}

std::vector<VkAttachmentReference>
IE::Graphics::RenderPass::buildReferencesFromSource(std::vector<std::string> &t_attachments) {
    std::vector<VkAttachmentReference> references(t_attachments.size());
    size_t                             i{};
    std::generate(references.begin(), references.end(), [&] {
        return *buildAttachmentReference(t_attachments[i]);
    });
    return references;
}

VkAttachmentReference *IE::Graphics::RenderPass::buildAttachmentReference(const std::string &t_attachment) {
    auto attachment{m_attachments.find(t_attachment)};
    return new VkAttachmentReference{
      .attachment = static_cast<uint32_t>(std::distance(m_attachments.begin(), attachment)),
      .layout     = dynamic_cast<IE::Graphics::detail::AttachmentVulkan *>(attachment->second.get())->m_layout,
    };
}

bool IE::Graphics::RenderPass::isRequired(const std::string &t_attachment) {
    if (std::find(m_outputAttachments.begin(), m_outputAttachments.end(), t_attachment) != m_outputAttachments.end())
        return true;
    for (auto subpass : m_subpasses)
        if (std::find(subpass.m_requiredAttachments.begin(),
                  subpass.m_requiredAttachments.end(),
                  t_attachment) != subpass.m_requiredAttachments.end())
            return true;
    return false;
}

// REQUIRES ATTACHMENT DESCRIPTIONS FOR ALL PRIOR SUBPASSES TO ALREADY BE COMPUTED.
VkAttachmentDescription
IE::Graphics::RenderPass::buildAttachmentDescriptionForSubpass(size_t i, const std::string &t_attachment) {
    size_t attachmentHistoryIndex{
      static_cast<size_t>(std::distance(m_attachments.begin(), m_attachments.find(t_attachment)))};
    std::shared_ptr<IE::Graphics::Attachment> image{m_attachments.at(t_attachment)};
    VkAttachmentDescription                   description{
                        .flags   = 0x0,
                        .format  = dynamic_cast<IE::Graphics::detail::AttachmentVulkan *>(image.get())->m_format,
                        .samples = dynamic_cast<IE::Graphics::detail::AttachmentVulkan *>(image.get())->m_samples,
                        .loadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                        .stencilLoadOp  = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                        .initialLayout  = VK_IMAGE_LAYOUT_UNDEFINED,
                        .finalLayout    = VK_IMAGE_LAYOUT_UNDEFINED};

    // Decide if we care about the previous contents of this attachment. If we do,
    // fill in the appropriate loadOp, and set the initialLayout.
    IE::Graphics::Subpass &subpass{*findNextAttachmentUseAfter(0, t_attachment)};
    if (std::find(subpass.m_requiredAttachments.begin(), subpass.m_requiredAttachments.end(), t_attachment) != m_subpasses[i].m_requiredAttachments.end()) {
        // Attachment contents required
        if (subpass.m_depthStencilAttachment == t_attachment)
            description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        else description.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        description.initialLayout = m_attachmentStateHistory[attachmentHistoryIndex].finalLayout;
    }

    // Is storage required?
    if (isRequired(t_attachment)) {
        if (subpass.m_depthStencilAttachment == t_attachment)
            description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        else description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        description.finalLayout =
          IE::Graphics::detail::AttachmentVulkan::layoutFromPreset.at(subpass.attachmentPresets.at(t_attachment));
    }

    m_attachmentStateHistory[attachmentHistoryIndex] = description;

    return description;
}

IE::Graphics::RenderPass::RenderPass(std::weak_ptr<IE::Graphics::RenderEngine> t_engineLink) :
        m_linkedRenderEngine(std::move(t_engineLink)) {
}

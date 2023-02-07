#pragma once

#include "GraphicsModule/DescriptorSet/DescriptorSet.hpp"
#include "Pipeline.hpp"
#include "Renderable/Renderable.hpp"
#include "Shader.hpp"

#include <Image/Image.hpp>
#include <string>
#include <unordered_map>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace IE::Graphics {
class RenderPass;

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

    struct AttachmentDescription {
        AttachmentConsumption       m_consumption;
        IE::Graphics::Image::Preset m_preset;
    } __attribute__((aligned(8)));

    enum Preset {
        IE_SUBPASS_PRESET_CUSTOM = 0x0,
    };

    explicit Subpass(Preset t_preset, std::vector<std::shared_ptr<Shader>> &t_shader);

    ~Subpass();

    IE::Graphics::RenderEngine *m_linkedRenderEngine;

    // Building
    Preset                               m_preset;
    RenderPass                          *m_renderPass{};
    Pipeline                             pipeline;
    std::vector<std::shared_ptr<Shader>> shaders;
    DescriptorSet                        descriptorSet{DescriptorSet::IE_DESCRIPTOR_SET_TYPE_PER_SUBPASS};
    std::vector<std::pair<std::string, AttachmentDescription>> m_attachments{};
    std::vector<VkAttachmentReference>                         m_input;
    std::vector<VkAttachmentReference>                         m_color;
    std::vector<VkAttachmentReference>                         m_resolve;
    std::vector<VkAttachmentReference>                         m_depth;
    std::vector<uint32_t>                                      m_preserve;

    auto addOrModifyAttachment(
      const std::string                           &t_attachmentName,
      IE::Graphics::Subpass::AttachmentConsumption t_consumption,
      Image::Preset                                t_type
    ) -> decltype(*this);

    void build(IE::Graphics::RenderPass *t_renderPass);

    // Executing
    std::vector<std::shared_ptr<IE::Graphics::CommandBuffer>> m_commandBuffers{};

    void registerRenderable(IE::Graphics::Renderable *t_renderable);

    void destroy();

    void execute();
    void execute(CommandBuffer t_masterCommandBuffer);
};
}  // namespace IE::Graphics
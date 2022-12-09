#pragma once

#include "GraphicsModule/RenderPass/Shader.hpp"

namespace IE::Graphics {
class Subpass;

class DescriptorSet {
public:
    enum SetNumber {
        IE_DESCRIPTOR_SET_TYPE_PER_FRAME,
        IE_DESCRIPTOR_SET_TYPE_PER_SUBPASS,
        IE_DESCRIPTOR_SET_TYPE_PER_MATERIAL,
        IE_DESCRIPTOR_SET_TYPE_PER_OBJECT
    };

    IE::Graphics::Subpass *m_subpass;
    VkDescriptorPool       m_pool;
    VkDescriptorSet        m_set;
    SetNumber              m_setNumber;

    DescriptorSet(SetNumber t_type);

    void build(IE::Graphics::Subpass *t_subpass, std::vector<std::shared_ptr<IE::Graphics::Shader>> &t_shaders);

    void build(RenderEngine *t_engineLink);

    static VkDescriptorSetLayout getLayout(
      IE::Graphics::RenderEngine          *t_engineLink,
      size_t                               t_set,
      std::vector<std::shared_ptr<Shader>> t_shaders
    );

private:
    static const std::vector<std::tuple<VkDescriptorType, uint32_t>> PER_FRAME_DESCRIPTOR_SET_LAYOUT_INFO;
};
}  // namespace IE::Graphics
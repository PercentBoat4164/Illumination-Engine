#pragma once

#include "GraphicsModule/RenderPass/Shader.hpp"

#include <vulkan/vulkan.hpp>

namespace IE::Graphics {
class Subpass;

class DescriptorSet {
public:
    enum SetType {
        IE_DESCRIPTOR_SET_TYPE_PER_FRAME,
        IE_DESCRIPTOR_SET_TYPE_PER_SUBPASS,
        IE_DESCRIPTOR_SET_TYPE_PER_MATERIAL,
        IE_DESCRIPTOR_SET_TYPE_PER_OBJECT
    };

    VkDescriptorPool m_pool;
    VkDescriptorSet  m_set;
    SetType          m_type;
    RenderEngine    *m_linkedRenderEngine;

    DescriptorSet(SetType t_type);

    ~DescriptorSet();

    void build(IE::Graphics::Subpass *t_subpass, std::vector<std::shared_ptr<IE::Graphics::Shader>> &t_shaders);

    void build(RenderEngine *t_engineLink);

    static VkDescriptorSetLayout getLayout(
      IE::Graphics::RenderEngine          *t_engineLink,
      size_t                               t_set,
      std::vector<std::shared_ptr<Shader>> t_shaders
    );

    void destroy();

private:
    static const std::vector<std::tuple<VkDescriptorType, uint32_t>> PER_FRAME_DESCRIPTOR_SET_LAYOUT_INFO;
};
}  // namespace IE::Graphics
#pragma once

#include "GraphicsModule/RenderPass/Shader.hpp"

namespace IE::Graphics {
class Subpass;

class DescriptorSet {
public:
    enum Type {
        IE_DESCRIPTOR_SET_TYPE_PER_FRAME,
        IE_DESCRIPTOR_SET_TYPE_PER_SUBPASS,
        IE_DESCRIPTOR_SET_TYPE_PER_MATERIAL,
        IE_DESCRIPTOR_SET_TYPE_PER_OBJECT
    };

    IE::Graphics::Subpass *m_subpass;
    VkDescriptorPool       m_pool;
    Type                   m_type;

    DescriptorSet(Type t_type);

    void build(IE::Graphics::Subpass *t_subpass, std::vector<IE::Graphics::Shader> &t_shaders);

    void build(RenderEngine *t_engineLink);

    static bool isDescriptorControlledBySetType(Type t_type, std::string name);

private:
    static const std::array<std::vector<std::string>, 4>          DESCRIPTOR_TYPE_MAP;
    static const std::vector<std::string>                         PER_FRAME_DESCRIPTORS;
    static const std::vector<std::string>                         PER_SUBPASS_DESCRIPTORS;
    static const std::vector<std::string>                         PER_MATERIAL_DESCRIPTORS;
    static const std::vector<std::string>                         PER_OBJECT_DESCRIPTORS;
    static const std::array<std::vector<VkDescriptorPoolSize>, 4> DESCRIPTOR_POOL_SIZES;
};
}  // namespace IE::Graphics
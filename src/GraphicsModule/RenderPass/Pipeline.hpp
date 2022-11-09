#pragma once

#include "Shader.hpp"

#include <vulkan/vulkan_core.h>

namespace IE::Graphics {
class Subpass;

class Pipeline {
public:
    VkPipeline       m_pipeline{};
    VkPipelineLayout m_layout{};
    VkPipelineCache  m_cache{VK_NULL_HANDLE};
    Subpass         *m_subpass;

    explicit Pipeline(Subpass *t_subpass);

    void build(IE::Graphics::Subpass *t_subpass, const std::vector<IE::Graphics::Shader> &t_shaders);
};
}  // namespace IE::Graphics
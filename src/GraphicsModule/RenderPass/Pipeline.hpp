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

    Pipeline();

    void
    build(IE::Graphics::Subpass *t_subpass, const std::vector<std::shared_ptr<IE::Graphics::Shader>> &t_shaders);
};
}  // namespace IE::Graphics
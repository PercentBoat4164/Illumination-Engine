#pragma once

#include <vulkan/vulkan_core.h>

namespace IE::Graphics {
class Subpass;

class Pipeline {
    VkPipeline             m_pipeline{};
    VkPipelineCache        m_cache{VK_NULL_HANDLE};
    IE::Graphics::Subpass *m_subpass;

    void build();

public:
    Pipeline(Subpass *t_subpass);
};
}  // namespace IE::Graphics
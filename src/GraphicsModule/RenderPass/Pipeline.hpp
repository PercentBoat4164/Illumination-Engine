#pragma once

#include <vulkan/vulkan_core.h>

namespace IE::Graphics {
class Pipeline {
    VkPipeline m_pipeline;

    void build() {
        VkGraphicsPipelineCreateInfo pipelineCreateInfo{
          .sType   = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
          .pNext   = nullptr,
          .flags   = 0x0,
          .subpass = 0x0};
    }
};
}  // namespace IE::Graphics
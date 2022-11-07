#pragma once

#include <filesystem>
#include <shaderc/shaderc.hpp>
#include <vulkan/vulkan.hpp>

namespace IE::Graphics {
class RenderEngine;

class Shader {
public:
    std::string                 contents;
    std::string                 filename;
    shaderc_shader_kind         kind;
    VkShaderStageFlagBits       stage;
    VkShaderModule              module;
    IE::Graphics::RenderEngine *m_linkedRenderEngine{};

    void compile();

    explicit Shader(const std::filesystem::path &t_filename);
};
}  // namespace IE::Graphics
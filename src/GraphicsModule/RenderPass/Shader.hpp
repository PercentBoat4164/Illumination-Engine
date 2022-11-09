#pragma once

#include <filesystem>
#include <shaderc/shaderc.hpp>
#include <vulkan/vulkan.hpp>

namespace IE::Graphics {
class RenderEngine;
class Subpass;

class Shader {
public:
    std::string            m_contents;
    std::filesystem::path  m_filename;
    shaderc_shader_kind    m_kind;
    VkShaderStageFlagBits  m_stage;
    VkShaderModule         m_module{};
    IE::Graphics::Subpass *m_subpass{};

    void compile();

    void reflect();

    explicit Shader(const std::filesystem::path &t_filename);
    void build(Subpass *t_subpass);
};
}  // namespace IE::Graphics
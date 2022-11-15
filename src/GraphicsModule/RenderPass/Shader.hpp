#pragma once

#include <filesystem>
#include <shaderc/shaderc.hpp>
#include <spirv-reflect/spirv_reflect.h>
#include <vulkan/vulkan.hpp>

namespace IE::Core {
class File;
}  // namespace IE::Core

namespace IE::Graphics {
class RenderEngine;
class Subpass;

class Shader {
public:
    IE::Core::File        *m_file;
    std::vector<uint32_t>  m_code;
    shaderc_shader_kind    m_kind;
    VkShaderStageFlagBits  m_stage;
    VkShaderModule         m_module{};
    IE::Graphics::Subpass *m_subpass{};

    void compile();

    spv_reflect::ShaderModule reflect();

    explicit Shader(const std::filesystem::path &t_filename);

    void build(Subpass *t_subpass);
};
}  // namespace IE::Graphics
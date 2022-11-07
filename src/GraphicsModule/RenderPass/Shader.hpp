#pragma once

#include <filesystem>
#include <shaderc/shaderc.hpp>

namespace IE::Graphics {
class RenderEngine;

class Shader {
    std::string                 contents;
    std::string                 filename;
    shaderc_shader_kind         kind;
    IE::Graphics::RenderEngine *m_linkedRenderEngine{};

    void compile();

public:
    explicit Shader(const std::filesystem::path &t_filename);
};
}  // namespace IE::Graphics
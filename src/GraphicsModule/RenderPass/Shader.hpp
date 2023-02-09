#pragma once

#include <filesystem>
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_reflect.hpp>
#include <vulkan/vulkan.hpp>

namespace IE::Core {
class File;
}  // namespace IE::Core

namespace IE::Graphics {
class RenderEngine;
class Subpass;

class Shader {
public:
    struct ReflectionInfo {
        std::string      name;
        uint32_t         binding;
        uint32_t         set;
        VkDescriptorType type;
    };

    IE::Core::File                                  *m_file;
    std::shared_ptr<spirv_cross::CompilerReflection> reflection;
    spirv_cross::ShaderResources                     reflectedData;
    std::vector<ReflectionInfo>                      reflectedInfo;
    bool                                             reflected{false};
    std::vector<uint32_t>                            m_code;
    shaderc_shader_kind                              m_kind;
    VkShaderStageFlagBits                            m_stage;
    VkShaderModule                                   m_module{};
    IE::Graphics::RenderEngine                      *m_linkedRenderEngine;

    void compile();

    spirv_cross::ShaderResources &reflect();

    explicit Shader(const std::filesystem::path &t_filename);

    std::vector<ReflectionInfo> getReflectionInfo();

    void build(IE::Graphics::RenderEngine *t_engineLink);

    void destroy();

    ~Shader();

    static VkShaderStageFlagBits stageFromExecutionModel(spv::ExecutionModel model);
};
}  // namespace IE::Graphics
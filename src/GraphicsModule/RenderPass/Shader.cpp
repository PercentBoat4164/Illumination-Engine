#include "Shader.hpp"

#include "Core/FileSystemModule/FileSystem.hpp"
#include "RenderEngine.hpp"

void IE::Graphics::Shader::compile() {
    // Compile
    shaderc::Compiler       compiler;
    shaderc::CompileOptions options;

    options.AddMacroDefinition("PER_FRAME_DESCRIPTOR", "0");
    options.AddMacroDefinition("PER_SUBPASS_DESCRIPTOR", "1");
    options.AddMacroDefinition("PER_MATERIAL_DESCRIPTOR", "2");
    options.AddMacroDefinition("PER_OBJECT_DESCRIPTOR", "3");

    options.SetOptimizationLevel(shaderc_optimization_level_performance);

    auto                          contents(m_file->read());
    std::string                   str(contents.begin(), contents.end());
    shaderc::SpvCompilationResult compilationResult{
      compiler.CompileGlslToSpv(str, m_kind, m_file->path.string().c_str(), options)};

    if (compilationResult.GetCompilationStatus() != shaderc_compilation_status_success)
        m_subpass->m_renderPass->m_renderPassSeries->m_linkedRenderEngine->getLogger().log(
          "Shader Compiler encountered " + std::to_string(compilationResult.GetNumErrors()) + " error(s) and " +
            std::to_string(compilationResult.GetNumWarnings()) + " warning(s) in " + m_file->path.string() +
            ":\n" + compilationResult.GetErrorMessage().erase(compilationResult.GetErrorMessage().length() - 1),
          Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    else
        m_subpass->m_renderPass->m_renderPassSeries->m_linkedRenderEngine->getLogger().log(
          "Compiled Shader " + m_file->path.string()
        );

    // reflect
    m_code = {compilationResult.begin(), compilationResult.end()};
    reflect();
    m_stage = static_cast<VkShaderStageFlagBits>(reflectedData->GetShaderStage());

    // Build module
    VkShaderModuleCreateInfo shaderModuleCreateInfo{
      .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .pNext    = nullptr,
      .flags    = 0x0,
      .codeSize = m_code.size() * 4,
      .pCode    = m_code.data()};
    VkResult result{vkCreateShaderModule(
      m_subpass->m_renderPass->m_renderPassSeries->m_linkedRenderEngine->m_device.device,
      &shaderModuleCreateInfo,
      nullptr,
      &m_module
    )};
    if (result != VK_SUCCESS) {
        m_subpass->m_renderPass->m_renderPassSeries->m_linkedRenderEngine->getLogger().log(
          "Failed to create shader module with error: " +
            IE::Graphics::RenderEngine::translateVkResultCodes(result),
          Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    }
    m_subpass->m_renderPass->m_renderPassSeries->m_linkedRenderEngine->getLogger().log("Created Shader Module");
}

IE::Graphics::Shader::Shader(const std::filesystem::path &t_filename) :
        m_file(IE::Core::Core::getFileSystem()->getFile(t_filename)) {
    if (m_file == nullptr) {
        IE::Core::Core::getLogger()->log(
          "File: " + t_filename.string() + " does not exist!",
          Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    }
    // Get kind from filename
    if (m_file->extension == ".vert") m_kind = shaderc_vertex_shader;
    else if (m_file->extension == ".tesc") m_kind = shaderc_tess_control_shader;
    else if (m_file->extension == ".tese") m_kind = shaderc_tess_evaluation_shader;
    else if (m_file->extension == ".geom") m_kind = shaderc_geometry_shader;
    else if (m_file->extension == ".frag") m_kind = shaderc_fragment_shader;
    else if (m_file->extension == ".comp") m_kind = shaderc_compute_shader;
    else if (m_file->extension == ".mesh") m_kind = shaderc_mesh_shader;
    else if (m_file->extension == ".task") m_kind = shaderc_task_shader;
    else if (m_file->extension == ".rgen") m_kind = shaderc_raygen_shader;
    else if (m_file->extension == ".ahit") m_kind = shaderc_anyhit_shader;
    else if (m_file->extension == ".chit") m_kind = shaderc_closesthit_shader;
    else if (m_file->extension == ".intr") m_kind = shaderc_intersection_shader;
    else if (m_file->extension == ".call") m_kind = shaderc_callable_shader;
    else if (m_file->extension == ".miss") m_kind = shaderc_miss_shader;
    else m_kind = shaderc_glsl_infer_from_source;
}

spv_reflect::ShaderModule &IE::Graphics::Shader::reflect() {
    if (!reflected) {
        reflected     = true;
        reflectedData = std::make_shared<spv_reflect::ShaderModule>(m_code, SPV_REFLECT_MODULE_FLAG_NO_COPY);
    }
    return *reflectedData;
}

void IE::Graphics::Shader::build(IE::Graphics::Subpass *t_subpass) {
    m_subpass = t_subpass;
}

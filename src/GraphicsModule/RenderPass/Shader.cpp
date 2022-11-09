#include "Shader.hpp"

#include "RenderEngine.hpp"

void IE::Graphics::Shader::compile() {
    // Compile
    shaderc::Compiler       compiler;
    shaderc::CompileOptions options;

    options.SetOptimizationLevel(shaderc_optimization_level_performance);

    shaderc::SpvCompilationResult compilationResult{
      compiler.CompileGlslToSpv(m_contents, m_kind, m_filename.c_str(), options)};

    if (compilationResult.GetCompilationStatus() != shaderc_compilation_status_success)
        m_subpass->m_renderPass->m_renderPassSeries->m_linkedRenderEngine->getLogger().log(
          "Shader Compiler encountered " + std::to_string(compilationResult.GetNumErrors()) + " error(s) and " +
            std::to_string(compilationResult.GetNumWarnings()) + " warning(s) in " + m_filename.string() + ":\n" +
            compilationResult.GetErrorMessage().erase(compilationResult.GetErrorMessage().length() - 1),
          Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    else
        m_subpass->m_renderPass->m_renderPassSeries->m_linkedRenderEngine->getLogger().log(
          "Compiled Shader " + m_filename.string()
        );

    // Build module
    std::vector<uint32_t>    code{compilationResult.begin(), compilationResult.end()};
    VkShaderModuleCreateInfo shaderModuleCreateInfo{
      .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .pNext    = nullptr,
      .flags    = 0x0,
      .codeSize = code.size() * 4,
      .pCode    = code.data()};
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

IE::Graphics::Shader::Shader(const std::filesystem::path &t_filename) : m_filename(t_filename) {
    // Get kind from filename
    std::string extension{t_filename.extension().string()};
    if (extension == ".vert") {
        m_kind  = shaderc_vertex_shader;
        m_stage = VK_SHADER_STAGE_VERTEX_BIT;
    } else if (extension == ".tesc") {
        m_kind  = shaderc_tess_control_shader;
        m_stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    } else if (extension == ".tese") {
        m_kind  = shaderc_tess_evaluation_shader;
        m_stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    } else if (extension == ".geom") {
        m_kind  = shaderc_geometry_shader;
        m_stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    } else if (extension == ".frag") {
        m_kind  = shaderc_fragment_shader;
        m_stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    } else if (extension == ".comp") {
        m_kind  = shaderc_compute_shader;
        m_stage = VK_SHADER_STAGE_COMPUTE_BIT;
    } else if (extension == ".mesh") {
        m_kind  = shaderc_mesh_shader;
        m_stage = VK_SHADER_STAGE_MESH_BIT_EXT;
    } else if (extension == ".task") {
        m_kind  = shaderc_task_shader;
        m_stage = VK_SHADER_STAGE_TASK_BIT_EXT;
    } else if (extension == ".rgen") {
        m_kind  = shaderc_raygen_shader;
        m_stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    } else if (extension == ".ahit") {
        m_kind  = shaderc_anyhit_shader;
        m_stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
    } else if (extension == ".chit") {
        m_kind  = shaderc_closesthit_shader;
        m_stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    } else if (extension == ".intr") {
        m_kind  = shaderc_intersection_shader;
        m_stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
    } else if (extension == ".call") {
        m_kind  = shaderc_callable_shader;
        m_stage = VK_SHADER_STAGE_CALLABLE_BIT_KHR;
    } else if (extension == ".miss") {
        m_kind  = shaderc_miss_shader;
        m_stage = VK_SHADER_STAGE_MISS_BIT_KHR;
    } else {
        m_kind  = shaderc_glsl_infer_from_source;
        m_stage = VK_SHADER_STAGE_ALL;
    }
}

void IE::Graphics::Shader::reflect() {
}

void IE::Graphics::Shader::build(IE::Graphics::Subpass *t_subpass) {
    m_subpass = t_subpass;

    // Read file
    IEFile file(m_filename);
    file.open();
    m_contents = file.read(file.length, 0);
}

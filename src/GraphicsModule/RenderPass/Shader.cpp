#include "Shader.hpp"

#include "RenderEngine.hpp"

void IE::Graphics::Shader::compile() {
    shaderc::Compiler       compiler;
    shaderc::CompileOptions options;
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
    shaderc::AssemblyCompilationResult result{
      compiler.CompileGlslToSpvAssembly(contents, kind, filename.c_str(), options)};
    if (result.GetCompilationStatus() != shaderc_compilation_status_success)
        m_linkedRenderEngine->getLogger().log(
          "Shader Compiler encountered " + std::to_string(result.GetNumErrors()) + " errors and " +
            std::to_string(result.GetNumWarnings()) + " warnings in file: " + filename + " Full error: \n" +
            result.GetErrorMessage(),
          Core::Logger::ILLUMINATION_ENGINE_LOG_LEVEL_ERROR
        );
    else m_linkedRenderEngine->getLogger().log("Compiled shader " + filename);
}

IE::Graphics::Shader::Shader(const std::filesystem::path &t_filename) {
    // Get kind from filename
    std::string extension{t_filename.extension().string()};
    if (extension == ".vert") {
        kind  = shaderc_vertex_shader;
        stage = VK_SHADER_STAGE_VERTEX_BIT;
    } else if (extension == ".tesc") {
        kind  = shaderc_tess_control_shader;
        stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
    } else if (extension == ".tese") {
        kind  = shaderc_tess_evaluation_shader;
        stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
    } else if (extension == ".geom") {
        kind  = shaderc_geometry_shader;
        stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    } else if (extension == ".frag") {
        kind  = shaderc_fragment_shader;
        stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    } else if (extension == ".comp") {
        kind  = shaderc_compute_shader;
        stage = VK_SHADER_STAGE_COMPUTE_BIT;
    } else if (extension == ".mesh") {
        kind  = shaderc_mesh_shader;
        stage = VK_SHADER_STAGE_MESH_BIT_EXT;
    } else if (extension == ".task") {
        kind  = shaderc_task_shader;
        stage = VK_SHADER_STAGE_TASK_BIT_EXT;
    } else if (extension == ".rgen") {
        kind  = shaderc_raygen_shader;
        stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
    } else if (extension == ".ahit") {
        kind  = shaderc_anyhit_shader;
        stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
    } else if (extension == ".chit") {
        kind  = shaderc_closesthit_shader;
        stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
    } else if (extension == ".intr") {
        kind  = shaderc_intersection_shader;
        stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
    } else if (extension == ".call") {
        kind  = shaderc_callable_shader;
        stage = VK_SHADER_STAGE_CALLABLE_BIT_KHR;
    } else if (extension == ".miss") {
        kind  = shaderc_miss_shader;
        stage = VK_SHADER_STAGE_MISS_BIT_KHR;
    } else {
        kind  = shaderc_glsl_infer_from_source;
        stage = VK_SHADER_STAGE_ALL;
    }
}
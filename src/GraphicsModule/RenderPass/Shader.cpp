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
    if (extension == ".vert") kind = shaderc_vertex_shader;
    else if (extension == ".tesc") kind = shaderc_tess_control_shader;
    else if (extension == ".tese") kind = shaderc_tess_evaluation_shader;
    else if (extension == ".geom") kind = shaderc_geometry_shader;
    else if (extension == ".frag") kind = shaderc_fragment_shader;
    else if (extension == ".comp") kind = shaderc_compute_shader;
    else if (extension == ".mesh") kind = shaderc_mesh_shader;
    else if (extension == ".task") kind = shaderc_task_shader;
    else if (extension == ".rgen") kind = shaderc_raygen_shader;
    else if (extension == ".ahit") kind = shaderc_anyhit_shader;
    else if (extension == ".chit") kind = shaderc_closesthit_shader;
    else if (extension == ".intr") kind = shaderc_intersection_shader;
    else if (extension == ".call") kind = shaderc_callable_shader;
    else if (extension == ".miss") kind = shaderc_miss_shader;
    else kind = shaderc_glsl_infer_from_source;
}
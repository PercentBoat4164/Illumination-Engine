#include "Shader.hpp"

#include "Core/FileSystemModule/FileSystem.hpp"
#include "RenderEngine.hpp"

#include <spirv_cross/spirv_cpp.hpp>

void IE::Graphics::Shader::compile() {
    // Compile
    shaderc::Compiler       compiler;
    shaderc::CompileOptions options;

    options.AddMacroDefinition("IE_PER_FRAME_DESCRIPTOR", "0");
    options.AddMacroDefinition("IE_PER_SUBPASS_DESCRIPTOR", "1");
    options.AddMacroDefinition("IE_PER_MATERIAL_DESCRIPTOR", "2");
    options.AddMacroDefinition("IE_PER_OBJECT_DESCRIPTOR", "3");
    options.AddMacroDefinition("IE_DESCRIPTOR_AUTO", "0xFFFFFFFF");

    options.AddMacroDefinition("IE_ENGINE_DATA", "set=0, binding=0");
    options.AddMacroDefinition("IE_CAMERA_DATA", "set=0, binding=1");
    options.AddMacroDefinition("IE_PERSPECTIVE_DATA", "set=1, binding=0");
    options.AddMacroDefinition("IE_DIFFUSE_TEXTURE", "set=2, binding=0");
    options.AddMacroDefinition("IE_SPECULAR_TEXTURE", "set=2, binding=1");
    options.AddMacroDefinition("IE_OBJECT_DATA", "set=3, binding=0");


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
    m_stage = stageFromExecutionModel(reflection->get_execution_model());

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

spirv_cross::ShaderResources &IE::Graphics::Shader::reflect() {
    if (!m_code.empty() && !reflected) {
        reflected     = true;
        reflection    = std::make_shared<spirv_cross::CompilerReflection>(m_code);
        reflectedData = reflection->get_shader_resources();
    }
    return reflectedData;
}

std::vector<IE::Graphics::Shader::ReflectionInfo> IE::Graphics::Shader::getReflectionInfo() {
    reflect();
    if (reflectedInfo.empty()) {
        for (size_t i{}; i < reflectedData.uniform_buffers.size(); ++i) {
            reflectedInfo.push_back(
              {.name    = reflection->get_name(reflectedData.uniform_buffers[i].id),
               .binding = reflection->get_decoration(reflectedData.uniform_buffers[i].id, spv::DecorationBinding),
               .set =
                 reflection->get_decoration(reflectedData.uniform_buffers[i].id, spv::DecorationDescriptorSet),
               .type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER}
            );
        }
        for (size_t i{}; i < reflectedData.storage_buffers.size(); ++i) {
            reflectedInfo.push_back(
              {.name    = reflection->get_name(reflectedData.storage_buffers[i].id),
               .binding = reflection->get_decoration(reflectedData.storage_buffers[i].id, spv::DecorationBinding),
               .set =
                 reflection->get_decoration(reflectedData.storage_buffers[i].id, spv::DecorationDescriptorSet),
               .type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER}
            );
        }
        for (size_t i{}; i < reflectedData.storage_images.size(); ++i) {
            reflectedInfo.push_back(
              {.name    = reflection->get_name(reflectedData.storage_images[i].id),
               .binding = reflection->get_decoration(reflectedData.storage_images[i].id, spv::DecorationBinding),
               .set = reflection->get_decoration(reflectedData.storage_images[i].id, spv::DecorationDescriptorSet),
               .type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE}
            );
        }
        for (size_t i{}; i < reflectedData.sampled_images.size(); ++i) {
            reflectedInfo.push_back(
              {.name    = reflection->get_name(reflectedData.sampled_images[i].id),
               .binding = reflection->get_decoration(reflectedData.sampled_images[i].id, spv::DecorationBinding),
               .set = reflection->get_decoration(reflectedData.sampled_images[i].id, spv::DecorationDescriptorSet),
               .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER}
            );
        }
        for (size_t i{}; i < reflectedData.subpass_inputs.size(); ++i) {
            reflectedInfo.push_back(
              {.name    = reflection->get_name(reflectedData.subpass_inputs[i].id),
               .binding = reflection->get_decoration(reflectedData.subpass_inputs[i].id, spv::DecorationBinding),
               .set = reflection->get_decoration(reflectedData.subpass_inputs[i].id, spv::DecorationDescriptorSet),
               .type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT}
            );
        }
        for (size_t i{}; i < reflectedData.acceleration_structures.size(); ++i) {
            reflectedInfo.push_back(
              {.name = reflection->get_name(reflectedData.acceleration_structures[i].id),
               .binding =
                 reflection->get_decoration(reflectedData.acceleration_structures[i].id, spv::DecorationBinding),
               .set = reflection->get_decoration(
                 reflectedData.acceleration_structures[i].id,
                 spv::DecorationDescriptorSet
               ),
               .type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR}
            );
        }
        for (size_t i{}; i < reflectedData.separate_images.size(); ++i) {
            reflectedInfo.push_back(
              {.name    = reflection->get_name(reflectedData.separate_images[i].id),
               .binding = reflection->get_decoration(reflectedData.separate_images[i].id, spv::DecorationBinding),
               .set =
                 reflection->get_decoration(reflectedData.separate_images[i].id, spv::DecorationDescriptorSet),
               .type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE}
            );
        }
        for (size_t i{}; i < reflectedData.separate_samplers.size(); ++i) {
            reflectedInfo.push_back(
              {.name = reflection->get_name(reflectedData.separate_samplers[i].id),
               .binding =
                 reflection->get_decoration(reflectedData.separate_samplers[i].id, spv::DecorationBinding),
               .set =
                 reflection->get_decoration(reflectedData.separate_samplers[i].id, spv::DecorationDescriptorSet),
               .type = VK_DESCRIPTOR_TYPE_SAMPLER}
            );
        }
        return reflectedInfo;
    }
}

void IE::Graphics::Shader::build(IE::Graphics::Subpass *t_subpass) {
    m_subpass = t_subpass;
}

VkShaderStageFlagBits IE::Graphics::Shader::stageFromExecutionModel(spv::ExecutionModel model) {
    switch (model) {
        case spv::ExecutionModelVertex: return VK_SHADER_STAGE_VERTEX_BIT;
        case spv::ExecutionModelTessellationControl: return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case spv::ExecutionModelTessellationEvaluation: return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case spv::ExecutionModelGeometry: return VK_SHADER_STAGE_GEOMETRY_BIT;
        case spv::ExecutionModelFragment: return VK_SHADER_STAGE_FRAGMENT_BIT;
        case spv::ExecutionModelGLCompute: return VK_SHADER_STAGE_COMPUTE_BIT;
        case spv::ExecutionModelTaskNV: return VK_SHADER_STAGE_TASK_BIT_NV;
        case spv::ExecutionModelMeshNV: return VK_SHADER_STAGE_MESH_BIT_NV;
        case spv::ExecutionModelRayGenerationKHR: return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        case spv::ExecutionModelIntersectionKHR: return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        case spv::ExecutionModelAnyHitKHR: return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        case spv::ExecutionModelClosestHitKHR: return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        case spv::ExecutionModelMissKHR: return VK_SHADER_STAGE_MISS_BIT_KHR;
        case spv::ExecutionModelCallableKHR: return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
        case spv::ExecutionModelTaskEXT: return VK_SHADER_STAGE_TASK_BIT_EXT;
        case spv::ExecutionModelMeshEXT: return VK_SHADER_STAGE_MESH_BIT_EXT;
        default: return VK_SHADER_STAGE_ALL;
    }
}

set(IlluminationEngineSourceGraphicsModuleVulkanSourceFiles
        vulkanAccelerationStructure.hpp
        vulkanBuffer.hpp
        vulkanCamera.hpp
        vulkanCommandBuffer.hpp
        vulkanDescriptorSet.hpp
        vulkanFramebuffer.hpp
        vulkanGraphicsEngineLink.hpp
        vulkanImage.hpp
        vulkanPipeline.hpp
        vulkanRenderable.hpp
        vulkanRenderEngine.hpp
        vulkanRenderPass.hpp
        vulkanSettings.hpp
        vulkanShader.hpp
        vulkanTexture.hpp
        vulkanUniformBufferObject.hpp
        vulkanVertex.hpp
        IEGMVulkanMain.cpp)
add_executable(IEGraphicsModuleVulkan ${IlluminationEngineSourceGraphicsModuleVulkanSourceFiles})
set_target_properties(IEGraphicsModuleVulkan PROPERTIES LINKER_LANGUAGE CXX)
add_subdirectory(DefaultShaders)
add_dependencies(IEGraphicsModuleVulkan IEGraphicsModuleVulkanShaders)
addAssImp(IEGraphicsModuleVulkan)
addGLEW(IEGraphicsModuleVulkan)
addGLM(IEGraphicsModuleVulkan)
addVulkan(IEGraphicsModuleVulkan)
addGLFW(IEGraphicsModuleVulkan)

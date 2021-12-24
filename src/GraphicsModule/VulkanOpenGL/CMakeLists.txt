set(IlluminationEngineSourceGraphicsModuleVulkanOpenGLSourceFiles
        IEAccelerationStructure.hpp
        IEBuffer.hpp
        IECamera.hpp
        IECommandBuffer.hpp
        IEDescriptorSet.hpp
        IEFramebuffer.hpp
        IEGraphicsLink.hpp
        IEImage.hpp
        IEPipeline.hpp
        IERenderable.hpp
        IERenderEngine.hpp
        IERenderPass.hpp
        IESettings.hpp
        IEShader.hpp
        IETexture.hpp
        IEUniformBufferObject.hpp
        IEVertex.hpp
        IEGMVulkanOpenGLMain.cpp)
add_executable(IEGraphicsModuleVulkanOpenGL ${IlluminationEngineSourceGraphicsModuleVulkanOpenGLSourceFiles})
target_include_directories(IEGraphicsModuleVulkanOpenGL PUBLIC ${ILLUMINATION_ENGINE_BASE_DIRECTORY}/src)
set_target_properties(IEGraphicsModuleVulkanOpenGL PROPERTIES LINKER_LANGUAGE CXX)
add_subdirectory(DefaultShaders)
add_dependencies(IEGraphicsModuleVulkanOpenGL IEGraphicsModuleVulkanOpenGLShaders)
addAssImp(IEGraphicsModuleVulkanOpenGL)
addGLEW(IEGraphicsModuleVulkanOpenGL)
addGLM(IEGraphicsModuleVulkanOpenGL)
addVulkan(IEGraphicsModuleVulkanOpenGL)
addGLFW(IEGraphicsModuleVulkanOpenGL)
addSpdlog(IEGraphicsModuleVulkanOpenGL)

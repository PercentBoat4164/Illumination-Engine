add_subdirectory(DefaultShaders)  # Generate all subdirectories

set(IlluminationEngineSourceGraphicsModuleVulkanOpenGLSourceFiles  # Gather sources
        IEAccelerationStructure.hpp
        IEBuffer.hpp
        IECamera.hpp
        IECommandBuffer.hpp
        IEDescriptorSet.hpp
        IEFramebuffer.hpp
        IEGraphicsEngineLink.hpp
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
        )

# Create and define properties for the library
add_library(IEGraphicsModuleOpenGLVulkan ${IlluminationEngineSourceGraphicsModuleVulkanOpenGLSourceFiles})
set_target_properties(IEGraphicsModuleOpenGLVulkan PROPERTIES LINKER_LANGUAGE CXX)

#add_dependencies(IEGraphicsModuleOpenGLVulkan PUBLIC IEGraphicsModuleVulkanOpenGLShaders)  # Add shader pre-compilation as a dependency

# Add external dependency libraries and include directories to the target
target_link_libraries(IEGraphicsModuleOpenGLVulkan PUBLIC vk-bootstrap glfw glew spdlog assimp glm)
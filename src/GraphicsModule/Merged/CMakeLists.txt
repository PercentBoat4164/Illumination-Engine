add_subdirectory(DefaultShaders)  # Generate all subdirectories

set(IEGraphicsModuleMergedSourceFiles
        IEBuffer.hpp
        IECamera.hpp
        IECommandPool.hpp
        IEDescriptorSet.hpp
        IEFramebuffer.hpp
        IEImage.hpp
        IEPipeline.hpp
        IERenderEngine.hpp
        IERenderable.hpp
        IERenderEngineLink.hpp
        IERenderPass.hpp
        IESettings.hpp
        IEShader.hpp
        IETexture.hpp
        IEFramebufferAttachment.hpp
        IEVertex.hpp
        IEMesh.hpp
        IEVersion.hpp
        IEGPUData.hpp
        )

# Create and define properties for the library
add_library(IEGraphicsModuleMerged ${IEGraphicsModuleMergedSourceFiles})
set_target_properties(IEGraphicsModuleMerged PROPERTIES LINKER_LANGUAGE CXX)

add_dependencies(IEGraphicsModuleMerged PUBLIC IEGraphicsModuleShaders)  # Add shader pre-compilation as a dependency

# Add external dependency libraries and include directories to the target
target_link_libraries(IEGraphicsModuleOpenGLVulkan PUBLIC vk-bootstrap glfw glew spdlog assimp glm)
add_subdirectory(DefaultShaders)  # Generate all subdirectories

set(IlluminationEngineSourceGraphicsModuleVulkanOpenGLSourceFiles  # Gather sources
        IEAccelerationStructure.hpp
        IEBuffer.hpp
        IECamera.hpp
        IEDescriptorSet.hpp
        IEFramebuffer.hpp
        IEGraphicsLink.hpp
        IEImage.hpp
        IEMonitor.hpp
        IEPipeline.hpp
        IERenderEngine.hpp
        IERenderPass.hpp
        IERenderable.hpp
        IESettings.hpp
        IEShader.hpp
        IETexture.hpp
        IEUniformBufferObject.hpp
        IEVertex.hpp
        IEMonitor.hpp IERenderableSettings.hpp)

# Create and define properties for the library
add_library(IEGraphicsModuleVulkanOpenGL ${IlluminationEngineSourceGraphicsModuleVulkanOpenGLSourceFiles})
target_include_directories(IEGraphicsModuleVulkanOpenGL PUBLIC ${ILLUMINATION_ENGINE_BASE_DIRECTORY}/src)
set_target_properties(IEGraphicsModuleVulkanOpenGL PROPERTIES LINKER_LANGUAGE CXX)

add_dependencies(IEGraphicsModuleVulkanOpenGL IEGraphicsModuleVulkanOpenGLShaders)  # Add shader pre-compilation as a dependency

# Add external dependency libraries and include directories to the target
if (Vulkan_FOUND)
    target_link_libraries(IEGraphicsModuleVulkanOpenGL PUBLIC glfw glew spdlog assimp glm vk-bootstrap vulkan)
    target_include_directories(IEGraphicsModuleVulkanOpenGL PUBLIC ${glew_DIR} ${glfw_DIR} ${glm_DIR} ${stb_image_DIR} ${assimp_DIR} ${vk-bootstrap_DIR} ${vma_DIR})
    target_compile_definitions(IEGraphicsModuleVulkanOpenGL PUBLIC ILLUMINATION_ENGINE_VULKAN ILLUMINATION_ENGINE_OPENGL)  # Compile with Vulkan and OpenGL support
else ()
    target_link_libraries(IEGraphicsModuleVulkanOpenGL PUBLIC glfw glew spdlog assimp glm)
    target_include_directories(IEGraphicsModuleVulkanOpenGL PUBLIC ${glew_DIR} ${glfw_DIR} ${glm_DIR} ${stb_image_DIR} ${assimp_DIR})
    target_compile_definitions(IEGraphicsModuleVulkanOpenGL PUBLIC ILLUMINATION_ENGINE_OPENGL)  # Compile with OpenGL support
endif ()
# Generate all subdirectories
add_subdirectory(DefaultShaders)

set(IlluminationEngineSourceGraphicsModuleVulkanOpenGLSourceFiles  # Gather sources
#        ${IlluminationEngineSourceGraphicsModuleVulkanOpenGLSourceFiles}
        IEAPI.cpp
        IEAPI.hpp
        IEAccelerationStructure.cpp
        IEAccelerationStructure.hpp
        IEBuffer.cpp
        IEBuffer.hpp
        IECamera.cpp
        IECamera.hpp
        IECommandPool.cpp
        IECommandPool.hpp
        IEDescriptorSet.cpp
        IEDescriptorSet.hpp
        IEMonitor.cpp
        IEMonitor.hpp
        IEPipeline.cpp
        IEPipeline.hpp
        IERenderable.cpp
        IERenderable.hpp
        IERenderableSettings.cpp
        IERenderableSettings.hpp
        IERenderEngine.cpp
        IERenderEngine.hpp
        IERenderPass.cpp
        IERenderPass.hpp
        IESettings.cpp
        IESettings.hpp
        IEShader.cpp
        IEShader.hpp
        IEUniformBufferObject.cpp
        IEUniformBufferObject.hpp
        IEVersion.cpp
        IEVersion.hpp
        IEVertex.cpp
        IEVertex.hpp
        Image/IEFramebuffer.cpp
        Image/IEFramebuffer.hpp
        Image/IEImage.cpp
        Image/IEImage.hpp
        Image/IETexture.cpp
        Image/IETexture.hpp
        )

# Create and define properties for the library
add_library(IEGraphicsModuleVulkanOpenGL ${IlluminationEngineSourceGraphicsModuleVulkanOpenGLSourceFiles})
target_include_directories(IEGraphicsModuleVulkanOpenGL PUBLIC ${ILLUMINATION_ENGINE_BASE_DIRECTORY}/src ${CMAKE_CURRENT_SOURCE_DIR})
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
# Generate all subdirectories
add_subdirectory(Shaders)

set(IEGraphicsModuleSourceFiles  # Gather sources
        IEAPI.cpp
        IEAPI.hpp
        Buffer/IEAccelerationStructure.cpp
        Buffer/IEAccelerationStructure.hpp
        Buffer/IEBuffer.cpp
        Buffer/IEBuffer.hpp
        IECamera.cpp
        IECamera.hpp
        IEDescriptorSet.cpp
        IEDescriptorSet.hpp
        IEMonitor.cpp
        IEMonitor.hpp
        IEPipeline.cpp
        IEPipeline.hpp
        Renderable/IERenderable.cpp
        Renderable/IERenderable.hpp
        IERenderableSettings.cpp
        IERenderableSettings.hpp
        IERenderEngine.cpp
        IERenderEngine.hpp
        IERenderPass.cpp
        IERenderPass.hpp
        IESettings.cpp
        IESettings.hpp
        Renderable/IEShader.cpp
        Renderable/IEShader.hpp
        IEUniformBufferObject.cpp
        IEUniformBufferObject.hpp
        IEVersion.cpp
        IEVersion.hpp
        Renderable/IEVertex.cpp
        Renderable/IEVertex.hpp
        CommandBuffer/IECommandBuffer.cpp
        CommandBuffer/IECommandBuffer.hpp
        CommandBuffer/IECommandPool.cpp
        CommandBuffer/IECommandPool.hpp
        Image/IEFramebuffer.cpp
        Image/IEFramebuffer.hpp
        Image/IEImage.cpp
        Image/IEImage.hpp
        Image/IETexture.cpp
        Image/IETexture.hpp
        IEDependency.cpp
        IEDependency.hpp
        IEDependent.cpp
        IEDependent.hpp
        Renderable/IEMaterial.cpp
        Renderable/IEMaterial.hpp
        )

# Create and define properties for the library
add_library(IEGraphicsModule ${IEGraphicsModuleSourceFiles})
target_include_directories(IEGraphicsModule PUBLIC ${ILLUMINATION_ENGINE_BASE_DIRECTORY}/src ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(IEGraphicsModule PROPERTIES LINKER_LANGUAGE CXX)

# Add internal dependency libraries to the target
target_link_libraries(IEGraphicsModule INTERFACE IECore)

# Add external dependency libraries and include directories to the target
if (Vulkan_FOUND)
    target_link_libraries(IEGraphicsModule PUBLIC glfw glew spdlog assimp glm vk-bootstrap vulkan)
    target_include_directories(IEGraphicsModule PUBLIC ${glew_DIR} ${glfw_DIR} ${glm_DIR} ${stb_image_DIR} ${assimp_DIR} ${vk-bootstrap_DIR} ${vma_DIR})
    target_compile_definitions(IEGraphicsModule PUBLIC ILLUMINATION_ENGINE_VULKAN ILLUMINATION_ENGINE_OPENGL)  # Compile with Vulkan and OpenGL support
else ()
    target_link_libraries(IEGraphicsModule PUBLIC glfw glew spdlog assimp glm)
    target_include_directories(IEGraphicsModule PUBLIC ${glew_DIR} ${glfw_DIR} ${glm_DIR} ${stb_image_DIR} ${assimp_DIR})
    target_compile_definitions(IEGraphicsModule PUBLIC ILLUMINATION_ENGINE_OPENGL)  # Compile with OpenGL support
endif ()

add_dependencies(IEGraphicsModule IEGMShaders)  # Add shader pre-compilation as a dependency
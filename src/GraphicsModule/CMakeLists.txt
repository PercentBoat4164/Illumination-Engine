# Generate all subdirectories
add_subdirectory(DefaultShaders)

set(IEGraphicsModuleSourceFiles  # Gather sources
        Buffer/IEBuffer.cpp
        Buffer/IEBuffer.hpp
        CommandBuffer/DependencyStructs/IEBufferMemoryBarrier.cpp
        CommandBuffer/DependencyStructs/IEBufferMemoryBarrier.hpp
        CommandBuffer/DependencyStructs/IECopyBufferToImageInfo.cpp
        CommandBuffer/DependencyStructs/IECopyBufferToImageInfo.hpp
        CommandBuffer/DependencyStructs/IEDependencyInfo.cpp
        CommandBuffer/DependencyStructs/IEDependencyInfo.hpp
        CommandBuffer/DependencyStructs/IEImageMemoryBarrier.cpp
        CommandBuffer/DependencyStructs/IEImageMemoryBarrier.hpp
        CommandBuffer/DependencyStructs/IERenderPassBeginInfo.cpp
        CommandBuffer/DependencyStructs/IERenderPassBeginInfo.hpp
        CommandBuffer/IECommandBuffer.cpp
        CommandBuffer/IECommandBuffer.hpp
        CommandBuffer/IECommandPool.cpp
        CommandBuffer/IECommandPool.hpp
		RenderPass/IEFramebuffer.cpp
		RenderPass/IEFramebuffer.hpp
        Image/IEImage.cpp
        Image/IEImage.hpp
        Image/IETexture.cpp
        Image/IETexture.hpp
        Renderable/IEMaterial.cpp
        Renderable/IEMaterial.hpp
        Renderable/IEMesh.cpp
        Renderable/IEMesh.hpp
        Renderable/IERenderable.cpp
        Renderable/IERenderable.hpp
		Shader/IEShader.cpp
		Shader/IEShader.hpp
        Renderable/IEVertex.cpp
        Renderable/IEVertex.hpp
        IEAPI.cpp
        IEAPI.hpp
        IECamera.cpp
        IECamera.hpp
		CommandBuffer/IEDependency.cpp
		CommandBuffer/IEDependency.hpp
		CommandBuffer/IEDependent.cpp
		CommandBuffer/IEDependent.hpp
		Shader/IEDescriptorSet.cpp
		Shader/IEDescriptorSet.hpp
        IEMonitor.cpp
        IEMonitor.hpp
		Shader/IEPipeline.cpp
		Shader/IEPipeline.hpp
        IERenderEngine.cpp
        IERenderEngine.hpp
		RenderPass/IERenderPass.cpp
		RenderPass/IERenderPass.hpp
        IERenderableSettings.cpp
        IERenderableSettings.hpp
        IESettings.cpp
        IESettings.hpp
		Shader/IEUniformBufferObject.cpp
		Shader/IEUniformBufferObject.hpp
        IEVersion.cpp
        IEVersion.hpp
		RenderPass/SubPass.cpp RenderPass/SubPass.hpp)

# Create and define properties for the library
add_library(IEGraphicsModule ${IEGraphicsModuleSourceFiles})
target_include_directories(IEGraphicsModule PUBLIC ${ILLUMINATION_ENGINE_BASE_DIRECTORY}/src ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(IEGraphicsModule PROPERTIES LINKER_LANGUAGE CXX)

# Add internal dependency libraries to the target
target_link_libraries(IEGraphicsModule INTERFACE IECore)

# Add external dependency libraries and include directories to the target
target_link_libraries(IEGraphicsModule PUBLIC glfw glew spdlog assimp glm)
target_include_directories(IEGraphicsModule PUBLIC ${glew_DIR} ${glfw_DIR} ${glm_DIR} ${stb_image_DIR} ${assimp_DIR})
target_compile_definitions(IEGraphicsModule PUBLIC ILLUMINATION_ENGINE_OPENGL)  # Compile with OpenGL support
if (Vulkan_FOUND)
    target_link_libraries(IEGraphicsModule PUBLIC vk-bootstrap vulkan)
    target_include_directories(IEGraphicsModule PUBLIC ${vk-bootstrap_DIR} ${vma_DIR})
    target_compile_definitions(IEGraphicsModule PUBLIC ILLUMINATION_ENGINE_VULKAN)  # Compile with Vulkan support
endif ()

add_dependencies(IEGraphicsModule IEGMShaders)  # Add shader pre-compilation as a dependency
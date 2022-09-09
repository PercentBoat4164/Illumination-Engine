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
        CommandBuffer/IEDependency.cpp
        CommandBuffer/IEDependency.hpp
        CommandBuffer/IEDependent.cpp
        CommandBuffer/IEDependent.hpp
        IEAPI.cpp
        IEAPI.hpp
        IECamera.cpp
        IECamera.hpp
        IEMonitor.cpp
        IEMonitor.hpp
        IERenderEngine.cpp
        IERenderEngine.hpp
        IESettings.cpp
        IESettings.hpp
        IEVersion.cpp
        IEVersion.hpp
        Image/IEImage.cpp
        Image/IEImage.hpp
		Image/IETexture.cpp
        Image/IETexture.hpp
		RenderPass/IEFramebuffer.cpp
        RenderPass/IEFramebuffer.hpp
        RenderPass/IERenderPass.cpp
        RenderPass/IERenderPass.hpp
        RenderPass/SubPass.cpp
        RenderPass/SubPass.hpp
        Renderable/IEMaterial.cpp
        Renderable/IEMaterial.hpp
        Renderable/IEMesh.cpp
        Renderable/IEMesh.hpp
        Renderable/IERenderable.cpp
        Renderable/IERenderable.hpp
        Renderable/IEVertex.cpp
        Renderable/IEVertex.hpp
        Shader/IEDescriptorSet.cpp
        Shader/IEDescriptorSet.hpp
        Shader/IEPipeline.cpp
        Shader/IEPipeline.hpp
        Shader/IEShader.cpp
        Shader/IEShader.hpp
        Shader/IEUniformBufferObject.cpp
        Shader/IEUniformBufferObject.hpp
		Image/Image.cpp
		Image/Image.hpp
		Image/ImageOpenGL.cpp
		Image/ImageOpenGL.hpp
		Image/ImageVulkan.cpp
		Image/ImageVulkan.hpp
		Image/Texture.cpp Image/Texture.hpp Image/Attachment.cpp Image/Attachment.hpp Image/TextureOpenGL.cpp Image/TextureOpenGL.hpp Image/TextureVulkan.cpp Image/TextureVulkan.hpp Image/AttachmentOpenGL.cpp Image/AttachmentOpenGL.hpp Image/AttachmentVulkan.cpp Image/AttachmentVulkan.hpp)

# Create and define properties for the library
add_library(IEGraphicsModule ${IEGraphicsModuleSourceFiles})
target_include_directories(IEGraphicsModule PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(IEGraphicsModule PROPERTIES LINKER_LANGUAGE CXX)

# Add internal dependency libraries to the target
target_link_libraries(IEGraphicsModule PUBLIC IECore)

# Add external dependency libraries and include directories to the target
target_link_libraries(IEGraphicsModule PUBLIC glfw glew spdlog assimp glm vk-bootstrap ${Vulkan_LIBRARIES})
target_include_directories(IEGraphicsModule PUBLIC ${glew_DIR} ${glfw_DIR} ${glm_DIR} ${stb_image_DIR} ${assimp_DIR} ${vk-bootstrap_DIR} ${vma_DIR} ${Vulkan_INCLUDE_DIR})

add_dependencies(IEGraphicsModule IEGMShaders)  # Add shader pre-compilation as a dependency

add_executable(IEGMmain main.cpp)
target_link_libraries(IEGMmain PUBLIC IEGraphicsModule)
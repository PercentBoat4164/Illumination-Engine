# Generate all subdirectories
add_subdirectory(DefaultShaders)

set(IEGraphicsModuleSourceFiles  # Gather sources
        #        Buffer/IEBuffer.cpp
        #        CommandBuffer/IECommandBuffer.cpp
        #        CommandBuffer/DependencyStructs/IEBufferMemoryBarrier.cpp
        #        CommandBuffer/DependencyStructs/IECopyBufferToImageInfo.cpp
        #        CommandBuffer/DependencyStructs/IEDependencyInfo.cpp
        #        CommandBuffer/DependencyStructs/IEImageMemoryBarrier.cpp
        #        CommandBuffer/DependencyStructs/IERenderPassBeginInfo.cpp
        #        CommandBuffer/IECommandPool.cpp
#        Camera/IECamera.cpp
        #        Renderable/IEMaterial.cpp
        #        Renderable/IEMesh.cpp
        #        Renderable/IERenderable.cpp
        #        Renderable/IEVertex.cpp
        #        Shader/IEDescriptorSet.cpp
        #        Shader/IEPipeline.cpp
        #        Shader/IEShader.cpp
        #        Shader/IEUniformBufferObject.cpp
        RenderEngine.cpp
        API/API.cpp
        API/Version.cpp
        Image/Image.cpp
        Image/ImageOpenGL.cpp
        Image/ImageVulkan.cpp
        Image/Texture.cpp
        Image/TextureOpenGL.cpp
        Image/TextureVulkan.cpp
        RenderPass/Framebuffer.cpp
        RenderPass/Framebuffer.hpp
        RenderPass/RenderPass.cpp
        RenderPass/RenderPass.hpp
        RenderPass/Subpass.cpp
        RenderPass/Subpass.hpp)

# Create and define properties for the library
add_library(IEGraphicsModule ${IEGraphicsModuleSourceFiles})
target_include_directories(IEGraphicsModule PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(IEGraphicsModule PROPERTIES LINKER_LANGUAGE CXX)

# Add internal dependency libraries to the target
target_link_libraries(IEGraphicsModule PUBLIC IECore)

# Add external dependency libraries and include directories to the target
target_link_libraries(IEGraphicsModule PUBLIC glfw glew spdlog assimp glm vk-bootstrap::vk-bootstrap ${Vulkan_LIBRARIES})
target_include_directories(IEGraphicsModule PUBLIC ${glew_DIR} ${glfw_DIR} ${glm_DIR} ${stb_image_DIR} ${assimp_DIR} ${vma_DIR} ${Vulkan_INCLUDE_DIR} ${vk-bootstrap_DIR})

add_dependencies(IEGraphicsModule IEGMShaders)  # Add shader pre-compilation as a dependency

add_executable(IEGMmain main.cpp)
target_link_libraries(IEGMmain PUBLIC IEGraphicsModule)
# Generate all subdirectories
add_subdirectory(DefaultShaders)

set(IEGraphicsModuleSourceFiles  # Gather sources
		RenderEngine.cpp
		Buffer/Buffer.cpp
		CommandBuffer/CommandBuffer.cpp
		CommandBuffer/CommandPool.cpp
		#        Camera/IECamera.cpp
		#        Renderable/Material.cpp
		#        Renderable/IEMesh.cpp
		#        Renderable/IERenderable.cpp
		Renderable/Vertex.cpp
		RenderPass/DescriptorSet.cpp
		RenderPass/Pipeline.cpp
		RenderPass/Shader.cpp
		#        Shader/UniformBufferObject.cpp
		API/API.cpp
		API/Version.cpp
		#        Image/Image.cpp
		#        Image/ImageOpenGL.cpp
		#        Image/ImageVulkan.cpp
		#        Image/Texture.cpp
		#        Image/TextureOpenGL.cpp
		#        Image/TextureVulkan.cpp
		SyncObjects/Semaphore.cpp
		SyncObjects/Fence.cpp
		Buffer/BufferVulkan.cpp
		Buffer/Buffer.cpp
		Buffer/BufferOpenGL.cpp
		RenderPass/Framebuffer.cpp
		RenderPass/RenderPass.cpp
		RenderPass/RenderPassSeries.cpp
		RenderPass/Subpass.cpp
		Image/ImageVulkan.cpp)

# Create and define properties for the library
add_library(IEGraphicsModule ${IEGraphicsModuleSourceFiles})
target_include_directories(IEGraphicsModule PUBLIC ${IE_BASE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(IEGraphicsModule PROPERTIES LINKER_LANGUAGE CXX)

# Add internal dependency libraries to the target
target_link_libraries(IEGraphicsModule PUBLIC IECore)

# Add external dependency libraries and include directories to the target
target_link_libraries(IEGraphicsModule PUBLIC INT_src EXT_AssImp EXT_GLM EXT_GLFW EXT_OpenGL EXT_Vulkan)

add_dependencies(IEGraphicsModule IEGMShaders CopyResources)  # Add shader pre-compilation as a dependency
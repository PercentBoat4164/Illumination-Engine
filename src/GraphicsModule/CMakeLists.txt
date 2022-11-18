# Generate all subdirectories
add_subdirectory(DefaultShaders)

set(IEGraphicsModuleSourceFiles  # Gather sources
		API/API.cpp
		API/Version.cpp
		Buffer/Buffer.cpp
		Buffer/Buffer.cpp
		Buffer/BufferOpenGL.cpp
		Buffer/BufferVulkan.cpp
		CommandBuffer/CommandBuffer.cpp
		CommandBuffer/CommandPool.cpp
		Image/ImageVulkan.cpp
		RenderEngine.cpp
		DescriptorSet/DescriptorSet.cpp
		RenderPass/Framebuffer.cpp
		RenderPass/Pipeline.cpp
		RenderPass/RenderPass.cpp
		RenderPass/RenderPassSeries.cpp
		RenderPass/Shader.cpp
		RenderPass/Subpass.cpp
		Renderable/Vertex.cpp
		SyncObjects/Fence.cpp
		SyncObjects/Semaphore.cpp
		)

# Create and define properties for the library
add_library(IEGraphicsModule ${IEGraphicsModuleSourceFiles})
target_include_directories(IEGraphicsModule PUBLIC ${IE_BASE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(IEGraphicsModule PROPERTIES LINKER_LANGUAGE CXX)

# Add internal dependency libraries to the target
target_link_libraries(IEGraphicsModule PUBLIC IECore)

# Add external dependency libraries and include directories to the target
target_link_libraries(IEGraphicsModule PUBLIC INT_src EXT_AssImp EXT_GLM EXT_GLFW EXT_OpenGL EXT_Vulkan)

add_dependencies(IEGraphicsModule IEGMShaders CopyResources)  # Add shader pre-compilation as a dependency
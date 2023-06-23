set(IEGraphicsModuleSourceFiles  # Gather sources
        Buffer/IEBuffer.cpp
        CommandBuffer/DependencyStructs/IEBufferMemoryBarrier.cpp
        CommandBuffer/DependencyStructs/IECopyBufferToImageInfo.cpp
        CommandBuffer/DependencyStructs/IEDependencyInfo.cpp
        CommandBuffer/DependencyStructs/IEImageMemoryBarrier.cpp
        CommandBuffer/DependencyStructs/IERenderPassBeginInfo.cpp
        CommandBuffer/IECommandBuffer.cpp
        CommandBuffer/IECommandPool.cpp
        CommandBuffer/IEDependency.cpp
        CommandBuffer/IEDependent.cpp
        IEAPI.cpp
        IECamera.cpp
        IEMonitor.cpp
        IERenderEngine.cpp
        IESettings.cpp
        IEVersion.cpp
        Image/IEImage.cpp
        Image/IEImageNEW.cpp
        Image/IENDimensionalResizableImage.cpp
        Image/IETexture.cpp
        Image/IETextureNEW.cpp
        RenderPass/IEFramebuffer.cpp
        RenderPass/IERenderPass.cpp
        RenderPass/SubPass.cpp
        Renderable/IEMaterial.cpp
        Renderable/IEMesh.cpp
        Renderable/IERenderable.cpp
        Renderable/IEVertex.cpp
        Shader/IEDescriptorSet.cpp
        Shader/IEPipeline.cpp
        Shader/IEShader.cpp
        Shader/IEUniformBufferObject.cpp
        )

# Create and define properties for the library
add_library(IEGraphicsModule ${IEGraphicsModuleSourceFiles})
target_include_directories(IEGraphicsModule PUBLIC ${IE_BASE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(IEGraphicsModule PROPERTIES LINKER_LANGUAGE CXX)

# Add internal dependency libraries to the target
target_link_libraries(IEGraphicsModule PUBLIC IECore)

# Add external dependency libraries and include directories to the target
target_link_libraries(IEGraphicsModule PUBLIC INT_src EXT_AssImp EXT_OpenGL EXT_Vulkan)
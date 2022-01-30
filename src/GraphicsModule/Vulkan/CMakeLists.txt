add_subdirectory(DefaultShaders)  # Generate all subdirectories

set(IlluminationEngineSourceGraphicsModuleVulkanSourceFiles  # Gather sources
        vulkanAccelerationStructure.hpp
        vulkanBuffer.hpp
        vulkanCamera.hpp
        vulkanCommandBuffer.hpp
        vulkanDescriptorSet.hpp
        vulkanFramebuffer.hpp
        vulkanGraphicsEngineLink.hpp
        vulkanImage.hpp
        vulkanPipeline.hpp
        vulkanRenderable.hpp
        vulkanRenderEngine.hpp
        vulkanRenderPass.hpp
        vulkanSettings.hpp
        vulkanShader.hpp
        vulkanTexture.hpp
        vulkanUniformBufferObject.hpp
        vulkanVertex.hpp
        )

# Create and define properties for the library
add_library(IEGraphicsModuleVulkan ${IlluminationEngineSourceGraphicsModuleVulkanSourceFiles})
set_target_properties(IEGraphicsModuleVulkan PROPERTIES LINKER_LANGUAGE CXX)
target_compile_definitions(IEGraphicsModuleVulkan PUBLIC ILLUMINATION_ENGINE_VULKAN)  # Compile with Vulkan support

add_dependencies(IEGraphicsModuleVulkan IEGraphicsModuleVulkanShaders)  # Add shader pre-compilation as a dependency

# Add external dependency libraries and include directories to the target
target_link_libraries(IEGraphicsModuleVulkan PUBLIC glew glfw glm assimp vk-bootstrap vulkan)
target_include_directories(IEGraphicsModuleVulkan PUBLIC ${glew_DIR} ${glfw_DIR} ${glm_DIR} ${stb_image_DIR} ${assimp_DIR} ${vk-bootstrap_DIR} ${vma_DIR})
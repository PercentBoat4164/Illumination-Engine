function (addVulkan target)
    find_package(Vulkan)
    if (Vulkan_FOUND)
        file(TOUCH ${CMAKE_BINARY_DIR}/VulkanFOUND)
        add_subdirectory(${CMAKE_SOURCE_DIR}/deps/vk-bootstrap ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${target}/vk-boostrap)
        target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/deps)
        target_link_libraries(${target} PUBLIC vk-bootstrap ${VK_BOOTSTRAP_LIBRARIES} Vulkan::Vulkan)
    endif ()
endfunction ()

add_compile_definitions(ILLUMINATION_ENGINE_VULKAN)
set(IlluminationEngineSourceGraphicsModuleVulkanSourceFiles
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
        IEGMVulkanMain.cpp)
add_library(IEGraphicsModuleVulkan ${IlluminationEngineSourceGraphicsModuleVulkanSourceFiles})
set_target_properties(IEGraphicsModuleVulkan PROPERTIES LINKER_LANGUAGE CXX)
add_subdirectory(DefaultShaders)
add_dependencies(IEGraphicsModuleVulkan IEGraphicsModuleVulkanShaders)
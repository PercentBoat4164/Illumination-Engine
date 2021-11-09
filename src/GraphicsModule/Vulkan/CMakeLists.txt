function (addVulkan target)
    find_package(Vulkan)
    if ((NOT EXISTS ${CMAKE_BINARY_DIR}/VulkanFOUND) AND Vulkan_FOUND)
        add_subdirectory(${ILLUMINATION_ENGINE_BASE_DIRECTORY}/deps/vk-bootstrap ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/vk-boostrap)
    endif ()
    target_include_directories(${target} PUBLIC ${ILLUMINATION_ENGINE_BASE_DIRECTORY}/deps ${ILLUMINATION_ENGINE_BASE_DIRECTORY}/deps/vk-bootstrap/src)
    target_link_libraries(${target} PUBLIC vk-bootstrap Vulkan::Vulkan)
    add_dependencies(${target} vk-bootstrap Vulkan::Vulkan)
    add_compile_definitions(ILLUMINATION_ENGINE_VULKAN)
    file(TOUCH ${CMAKE_BINARY_DIR}/VulkanFOUND)
endfunction ()

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
add_executable(IEGraphicsModuleVulkan ${IlluminationEngineSourceGraphicsModuleVulkanSourceFiles})
set_target_properties(IEGraphicsModuleVulkan PROPERTIES LINKER_LANGUAGE CXX)
add_subdirectory(DefaultShaders)
add_dependencies(IEGraphicsModuleVulkan IEGraphicsModuleVulkanShaders)
addAssImp(IEGraphicsModuleVulkan)
addGLEW(IEGraphicsModuleVulkan)
addGLM(IEGraphicsModuleVulkan)
addVulkan(IEGraphicsModuleVulkan)
addGLFW(IEGraphicsModuleVulkan)

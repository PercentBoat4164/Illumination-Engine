# Add all subdirectories
add_subdirectory(Rasterize)
add_subdirectory(RayTrace)

add_custom_target(IEGraphicsModuleVulkanShaders)  # Wrapper target for all sub-targets

# Add all sub-targets as dependencies
add_dependencies(IEGraphicsModuleVulkanShaders IEGraphicsModuleVulkanShadersRayTracing)
add_dependencies(IEGraphicsModuleVulkanShaders IEGraphicsModuleVulkanShadersRasterizing)
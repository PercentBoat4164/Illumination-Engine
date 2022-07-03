# Add all subdirectories
add_subdirectory(OpenGL)
add_subdirectory(Vulkan)

add_custom_target(IEGMShaders)  # Wrapper target for all sub-targets

# Add all sub-targets as dependencies
add_dependencies(IEGMShaders IEGMVulkanShaders)
add_dependencies(IEGMShaders IEGMOpenGLShaders)

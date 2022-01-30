# Add all subdirectories.
add_subdirectory(OpenGL)
if (Vulkan_FOUND)
    add_subdirectory(Vulkan)
endif ()

# Create and define properties for the library
add_library(IEGraphicsModule INTERFACE)
set_target_properties(IEGraphicsModule PROPERTIES LINKER_LANGUAGE CXX)

# Add internal dependency libraries to the target
target_link_libraries(IEGraphicsModule INTERFACE IECore)
target_link_libraries(IEGraphicsModule INTERFACE IEGraphicsModuleOpenGL)
if (Vulkan_FOUND)
    target_link_libraries(IEGraphicsModule INTERFACE IEGraphicsModuleVulkan)
endif ()
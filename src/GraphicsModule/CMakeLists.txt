# Add all subdirectories.
add_subdirectory(VulkanOpenGL)

# Create and define properties for the library
add_library(IEGraphicsModule INTERFACE)
set_target_properties(IEGraphicsModule PROPERTIES LINKER_LANGUAGE CXX)

# Add internal dependency libraries to the target
target_link_libraries(IEGraphicsModule INTERFACE IECore)
target_link_libraries(IEGraphicsModule INTERFACE IEGraphicsModuleVulkanOpenGL)
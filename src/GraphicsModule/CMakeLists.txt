# Add all subdirectories.
add_subdirectory(OpenGL)
add_subdirectory(Vulkan)

# Create and define properties for the executable target
add_executable(IEGraphicsModule IEGraphicsModuleMain.cpp)
set_target_properties(IEGraphicsModule PROPERTIES LINKER_LANGUAGE CXX ENABLE_EXPORTS ON)

# Add internal dependency libraries to the target
target_link_libraries(IEGraphicsModule INTERFACE IEGraphicsModuleOpenGL)
target_link_libraries(IEGraphicsModule INTERFACE IEGraphicsModuleVulkan)
target_link_libraries(IEGraphicsModule INTERFACE IECore)
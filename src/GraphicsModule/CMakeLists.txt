# Add all subdirectories.
add_subdirectory(OpenGL)
if (Vulkan_FOUND)
    add_subdirectory(Vulkan)
endif ()

# Create and define properties for the executable target
add_executable(IEGraphicsModule IEGraphicsModuleMain.cpp)
set_target_properties(IEGraphicsModule PROPERTIES LINKER_LANGUAGE CXX ENABLE_EXPORTS ON)

# Add internal dependency libraries to the target
target_link_libraries(IEGraphicsModule PUBLIC IECore)
target_link_libraries(IEGraphicsModule PUBLIC IEGraphicsModuleOpenGL)
if (Vulkan_FOUND)
    target_link_libraries(IEGraphicsModule PUBLIC IEGraphicsModuleVulkan)
endif ()
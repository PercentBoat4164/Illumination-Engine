# Add all subdirectories
add_subdirectory(Core)
add_subdirectory(InputModule)
add_subdirectory(GraphicsModule)

# Create and define properties for the executable target
add_executable(IlluminationEngine main.cpp)
set_target_properties(IlluminationEngine PROPERTIES LINKER_LANGUAGE CXX)

# Add internal dependency libraries to the target
target_link_libraries(IlluminationEngine IEGraphicsModule IEInputModule)

# Ensure that building Illumination Engine also copies resources to the destination
add_dependencies(IlluminationEngine CopyResources)

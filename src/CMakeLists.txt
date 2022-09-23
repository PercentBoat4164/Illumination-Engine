# Add all subdirectories
add_subdirectory(Core)
add_subdirectory(InputModule)
add_subdirectory(GraphicsModule)

# Create and define properties for the executable target
add_executable(IlluminationEngine main.cpp GraphicsModule/RenderEngine.cpp GraphicsModule/RenderEngine.hpp)
set_target_properties(IlluminationEngine PROPERTIES LINKER_LANGUAGE CXX)

# Add internal dependency libraries to the target
target_link_libraries(IlluminationEngine PUBLIC IEGraphicsModule IEInputModule IECore)
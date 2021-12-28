set(IEInputModuleSourceFiles  # Gather sources
        IeKeyboard.hpp
        IEInputModuleMain.cpp
        )

# Create and define properties for the executable target
add_executable(IEInputModule ${IEInputModuleSourceFiles})
set_target_properties(IEInputModule PROPERTIES LINKER_LANGUAGE CXX ENABLE_EXPORTS ON)

# Add external dependency libraries and include directories to the target
target_link_libraries(IEInputModule PUBLIC glfw glew)
target_include_directories(IEInputModule PUBLIC ${glfwDIR} ${glewDIR})
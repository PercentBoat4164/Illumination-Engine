set(IEInputModuleSourceFiles  # Gather sources
        IEKeyboard.cpp
        IEKeyboard.hpp
        )

# Create and define properties for the library
add_library(IEInputModule ${IEInputModuleSourceFiles})
set_target_properties(IEInputModule PROPERTIES LINKER_LANGUAGE CXX)

# Add external dependency libraries and include directories to the target
target_link_libraries(IEInputModule PUBLIC glfw glew)

# Compiles without this line, but without it CLion thinks it cannot find the files.
target_include_directories(IEInputModule PUBLIC ${glew_DIR} ${ILLUMINATION_ENGINE_BASE_DIRECTORY}/src)
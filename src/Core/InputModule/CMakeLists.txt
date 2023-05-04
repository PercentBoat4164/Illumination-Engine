set(IEInputModuleSourceFiles  # Gather sources
        Keyboard.cpp
        InputHandler.cpp)

# Create and define properties for the library
add_library(IEInputModule ${IEInputModuleSourceFiles})
set_target_properties(IEInputModule PROPERTIES LINKER_LANGUAGE CXX)

# Add external dependency libraries and include directories to the target
target_link_libraries(IEInputModule PUBLIC INT_src EXT_OpenGL IECore)
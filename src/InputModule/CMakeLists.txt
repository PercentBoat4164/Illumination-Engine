set(IEInputModuleSourceFiles  # Gather sources
        IEKeyboard.cpp
        )

# Create and define properties for the library
add_library(IEInputModule ${IEInputModuleSourceFiles})
set_target_properties(IEInputModule PROPERTIES LINKER_LANGUAGE CXX)

# Add external dependency libraries and include directories to the target
target_link_libraries(IEInputModule PUBLIC EXT_OpenGL)
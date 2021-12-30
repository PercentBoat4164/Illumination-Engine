set(IELogModuleSourceFiles  # Gather sources
        IELogger.hpp
        )

# Create and define properties for the library
add_library(IELogModule ${IELogModuleSourceFiles})
set_target_properties(IELogModule PROPERTIES LINKER_LANGUAGE CXX)

# Add external dependency libraries and include directories to the target
target_link_libraries(IELogModule PUBLIC spdlog)
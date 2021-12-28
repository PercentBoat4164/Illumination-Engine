set(IELogModuleSourceFiles  # Gather sources
        IELogger.hpp
        IELogModuleMain.cpp
        )

# Create and define properties for the executable target
add_executable(IELogModule ${IELogModuleSourceFiles})
set_target_properties(IELogModule PROPERTIES LINKER_LANGUAGE CXX ENABLE_EXPORTS ON)

# Add external dependency libraries and include directories to the target
target_link_libraries(IELogModule PUBLIC spdlog)
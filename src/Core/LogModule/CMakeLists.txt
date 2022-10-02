set(IELogModuleSourceFiles  # Gather sources
        IELogger.cpp
        )

# Create and define properties for the library
add_library(IELogModule ${IELogModuleSourceFiles})
target_link_libraries(IELogModule PUBLIC spdlog)
target_include_directories(IELogModule PUBLIC ${INCLUDE_DIRS})
set_target_properties(IELogModule PROPERTIES LINKER_LANGUAGE CXX)
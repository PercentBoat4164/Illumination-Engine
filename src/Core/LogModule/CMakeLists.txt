set(IELogModuleSourceFiles  # Gather sources
        IELogger.cpp
        IELogger.hpp
        )

# Create and define properties for the library
add_library(IELogModule ${IELogModuleSourceFiles})
target_link_libraries(IELogModule PUBLIC spdlog)
target_include_directories(IELogModule PUBLIC ${spdlog_DIR})
set_target_properties(IELogModule PROPERTIES LINKER_LANGUAGE CXX)
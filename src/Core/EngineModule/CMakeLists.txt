set(IEEngineModuleSourceFiles  # Gather sources
        Engine.cpp
        Window.cpp
        )

# Create and define properties for the library
add_library(IEEngineModule ${IEEngineModuleSourceFiles})
target_link_libraries(IEEngineModule PUBLIC INT_src IEAssetModule)
set_target_properties(IEEngineModule PROPERTIES LINKER_LANGUAGE CXX)

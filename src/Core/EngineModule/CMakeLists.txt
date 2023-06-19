set(IEEngineModuleSourceFiles  # Gather sources
  Engine.cpp
  EventActionMapping.cpp
  )

# Create and define properties for the library
add_library(IEEngineModule ${IEEngineModuleSourceFiles})
target_link_libraries(IEEngineModule IEAssetModule IELogModule IEFileSystemModule)
set_target_properties(IEEngineModule PROPERTIES LINKER_LANGUAGE CXX)

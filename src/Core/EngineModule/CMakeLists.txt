set(IEEngineModuleSourceFiles  # Gather sources
  Engine.cpp
  Window.cpp
  )

# Create and define properties for the library
add_library(IEEngineModule ${IEEngineModuleSourceFiles})
#target_include_directories(IEEngineModule PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${ILLUMINATION_ENGINE_BASE_DIRECTORY})
set_target_properties(IEEngineModule PROPERTIES LINKER_LANGUAGE CXX)

# Add internal dependency libraries to the target
target_link_libraries(IEEngineModule PUBLIC INT_src IECore)

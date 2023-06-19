set(IELogModuleSourceFiles  # Gather sources
  Logger.cpp
  )

# Create and define properties for the library
add_library(IELogModule ${IELogModuleSourceFiles})
target_link_libraries(IELogModule INT_src EXT_SpdLog EXT_Vulkan)
set_target_properties(IELogModule PROPERTIES LINKER_LANGUAGE CXX)
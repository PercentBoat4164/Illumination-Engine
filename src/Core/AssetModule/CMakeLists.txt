set(IEAssetModuleSourceFiles  # Gather sources
		IEAspect.cpp
		IEAsset.cpp
		)

# Create and define properties for the executable target
add_library(IEAssetModule ${IEAssetModuleSourceFiles})
target_link_libraries(IEAssetModule PUBLIC INT_src EXT_GLM)
set_target_properties(IEAssetModule PROPERTIES LINKER_LANGUAGE CXX)
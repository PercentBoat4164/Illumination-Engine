set(IEAssetModuleSourceFiles  # Gather sources
        IEAsset.hpp
        IEAspect.hpp)

# Create and define properties for the executable target
add_library(IEAssetModule INTERFACE ${IEAssetModuleSourceFiles})
set_target_properties(IEAssetModule PROPERTIES LINKER_LANGUAGE CXX)
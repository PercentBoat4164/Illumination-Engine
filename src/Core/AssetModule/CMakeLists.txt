set(IEAssetModuleSourceFiles  # Gather sources
        IEAspect.cpp
        IEAspect.hpp
        IEAsset.cpp
        IEAsset.hpp
        )

# Create and define properties for the executable target
add_library(IEAssetModule ${IEAssetModuleSourceFiles})
target_link_libraries(IEAssetModule PUBLIC glm)
target_include_directories(IEAssetModule PUBLIC ${glm_DIR})
set_target_properties(IEAssetModule PROPERTIES LINKER_LANGUAGE CXX)
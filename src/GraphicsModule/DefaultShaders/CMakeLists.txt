# Add all subdirectories
add_subdirectory(Rasterize)

add_custom_target(IEGMShaders)  # Wrapper target for all sub-targets

# Add all sub-targets as dependencies
add_dependencies(IEGMShaders IEGMSRasterize)

add_subdirectory(Rasterize)  # Add all subdirectories

add_custom_target(IEGraphicsModuleOpenGLShaders)  # Wrapper target for all sub-targets

add_dependencies(IEGraphicsModuleOpenGLShaders IEGraphicsModuleOpenGLShadersRasterizing)  # Add all sub-targets as dependencies
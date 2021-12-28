set(IEDefaultOpenGLRasterizingShaders  # Gather sources
        fragmentShader.frag
        vertexShader.vert
        )

add_custom_target(IEGraphicsModuleOpenGLShadersRasterizing)  # Create the custom target

set(shader_output_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/OpenGL/Rasterizing)  # Define destination directory
file(MAKE_DIRECTORY ${shader_output_directory})  # Create destination directory

foreach (shader ${IEDefaultOpenGLRasterizingShaders})
    add_custom_command(TARGET IEGraphicsModuleOpenGLShadersRasterizing COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${shader} ${shader_output_directory}/${shader} DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${shader})  # Copy source file to output directory
endforeach ()

# Note that shaders are not compiled before being copied. The OpenGL implementation does not yet support precompiled shaders.
#todo Implement OpenGL support for precompiled shaders.
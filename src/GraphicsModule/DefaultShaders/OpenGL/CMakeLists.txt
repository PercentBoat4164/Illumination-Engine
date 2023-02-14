set(IEGMOpenGLShaderSourceFiles
        fragmentShader.frag
        vertexShader.vert
        macOS/fragmentShader.frag
        macOS/vertexShader.vert
        )

add_custom_target(IEGMOpenGLShaders)  # Create the custom target

set(shader_output_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/OpenGL) # Define destination directory
file(MAKE_DIRECTORY ${shader_output_directory})  # Create destination directory
foreach (shader ${IEGMOpenGLShaderSourceFiles})
    add_custom_command(TARGET IEGMOpenGLShaders COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${shader} ${shader_output_directory}/${shader} DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${shader})  # Copy source file to output directory
endforeach ()
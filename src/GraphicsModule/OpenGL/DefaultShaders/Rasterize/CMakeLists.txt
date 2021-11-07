add_custom_target(IEGraphicsModuleOpenGLShadersRasterizing)
set(IEDefaultOpenGLRasterizingShaders
        ${CMAKE_CURRENT_SOURCE_DIR}/fragmentShader.frag
        ${CMAKE_CURRENT_SOURCE_DIR}/vertexShader.vert)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/shaders/OpenGL/Rasterizing)
foreach (shader ${IEDefaultOpenGLRasterizingShaders})
    add_custom_command(TARGET IEGraphicsModuleOpenGLShadersRasterizing COMMAND ${CMAKE_COMMAND} -E copy "${shader}" ${CMAKE_BINARY_DIR}/bin/shaders/OpenGL/Rasterizing DEPENDS "${shader}")
endforeach ()
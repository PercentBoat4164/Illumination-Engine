add_custom_target(IEGraphicsModuleOpenGLShadersRasterizing)
set(IEDefaultOpenGLRasterizingShaders
        ${CMAKE_CURRENT_SOURCE_DIR}/fragmentShader.frag
        ${CMAKE_CURRENT_SOURCE_DIR}/vertexShader.vert)
add_custom_command(TARGET IEGraphicsModuleOpenGLShadersRasterizing COMMAND mkdir -p ${CMAKE_BINARY_DIR}/bin/shaders/OpenGL/Rasterizing)
foreach (shader ${IEDefaultOpenGLRasterizingShaders})
    add_custom_command(TARGET IEGraphicsModuleOpenGLShadersRasterizing COMMAND cp -rf "${shader}" ${CMAKE_BINARY_DIR}/bin/shaders/OpenGL/Rasterizing DEPENDS "${shader}")
endforeach ()
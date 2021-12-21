add_custom_target(IEGraphicsModuleShaders)
set(IEDefaultShaders
        ${CMAKE_CURRENT_SOURCE_DIR}/DEBUGNormals.frag
        ${CMAKE_CURRENT_SOURCE_DIR}/DEBUGNormals.vert)
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/bin/shaders)
foreach (shader ${IEDefaultOpenGLRasterizingShaders})
    add_custom_command(TARGET IEGraphicsModuleMergedShaders COMMAND ${CMAKE_COMMAND} -E copy "${shader}" ${CMAKE_BINARY_DIR}/bin/shaders DEPENDS "${shader}")
endforeach ()
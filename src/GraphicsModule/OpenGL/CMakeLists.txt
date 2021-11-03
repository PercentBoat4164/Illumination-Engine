function (addGLEW target)
    set(OpenGL_GL_PREFERENCE LEGACY)
    find_package(OpenGL)
    target_link_libraries(${target} PUBLIC OpenGL::GL)
    file(TOUCH ${CMAKE_BINARY_DIR}/OpenGLFOUND)
    if ((NOT GLEW_FOUND) AND EXISTS ${CMAKE_SOURCE_DIR}/deps/glew)
        add_subdirectory(${CMAKE_SOURCE_DIR}/deps/glew/build/cmake ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${target}/glew)
        target_include_directories(${target} PUBLIC ${CMAKE_SOURCE_DIR}/deps/glew/include)
        target_link_libraries(${target} PUBLIC glew ${glew_LIBRARIES})
        message("Building with packaged glew")
    else ()
        target_link_libraries(${target} PUBLIC GLEW ${glew_LIBRARIES})
    endif ()
    set(GLEW_FOUND ON)
endfunction ()

set(IlluminationEngineSourceGraphicsModuleOpenGLSourceFiles
        openglCamera.hpp
        openglFramebuffer.hpp
        openglGraphicsEngineLink.hpp
        openglImage.hpp
        openglProgram.hpp
        openglRenderable.hpp
        openglRenderEngine.hpp
        openglSettings.hpp
        openglShader.hpp
        openglTexture.hpp
        IEGMOpenGLMain.cpp
    )
add_library(IEGraphicsModuleOpenGL ${IlluminationEngineSourceGraphicsModuleOpenGLSourceFiles})
set_target_properties(IEGraphicsModuleOpenGL PROPERTIES LINKER_LANGUAGE CXX)
add_subdirectory(DefaultShaders)
add_dependencies(IEGraphicsModuleOpenGL IEGraphicsModuleOpenGLShaders)
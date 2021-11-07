function (addGLEW target)
    set(OpenGL_GL_PREFERENCE LEGACY)
    find_package(OpenGL REQUIRED)
    target_link_libraries(${target} PUBLIC OpenGL::GL)
    file(TOUCH ${CMAKE_BINARY_DIR}/OpenGLFOUND)
    if ((NOT EXISTS ${CMAKE_BINARY_DIR}/GLEWFOUND) AND EXISTS ${CMAKE_SOURCE_DIR}/deps/glew)
        add_subdirectory(${CMAKE_SOURCE_DIR}/deps/glew/build/cmake ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/glew)
    endif ()
    target_include_directories(${target} PUBLIC ${CMAKE_SOURCE_DIR}/deps/glew/include)
    target_link_libraries(${target} PUBLIC glew)
    add_dependencies(${target} glew)
    add_compile_definitions(ILLUMINATION_ENGINE_OPENGL)
    file(TOUCH ${CMAKE_BINARY_DIR}/GLEWFOUND)
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
        IEGMOpenGLMain.cpp)
add_executable(IEGraphicsModuleOpenGL ${IlluminationEngineSourceGraphicsModuleOpenGLSourceFiles})
set_target_properties(IEGraphicsModuleOpenGL PROPERTIES LINKER_LANGUAGE CXX)
add_subdirectory(DefaultShaders)
add_dependencies(IEGraphicsModuleOpenGL IEGraphicsModuleOpenGLShaders)
target_include_directories(IEGraphicsModuleOpenGL PUBLIC ${CMAKE_SOURCE_DIR}/src)
addAssImp(IEGraphicsModuleOpenGL)
addGLEW(IEGraphicsModuleOpenGL)
addGLM(IEGraphicsModuleOpenGL)
addGLFW(IEGraphicsModuleOpenGL)
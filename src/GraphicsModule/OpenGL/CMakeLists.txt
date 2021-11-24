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
target_include_directories(IEGraphicsModuleOpenGL PUBLIC ${ILLUMINATION_ENGINE_BASE_DIRECTORY}/src)
addAssImp(IEGraphicsModuleOpenGL)
addGLEW(IEGraphicsModuleOpenGL)
addGLM(IEGraphicsModuleOpenGL)
addGLFW(IEGraphicsModuleOpenGL)
if (OpenGL_FOUND)
    add_subdirectory(DefaultShaders)  # Generate all subdirectories

    set(IlluminationEngineSourceGraphicsModuleOpenGLSourceFiles  # Gather sources
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
            IEWindowUserPointer.hpp)

    # Create and define properties for the library
    add_library(IEGraphicsModuleOpenGL INTERFACE ${IlluminationEngineSourceGraphicsModuleOpenGLSourceFiles})
    set_target_properties(IEGraphicsModuleOpenGL PROPERTIES LINKER_LANGUAGE CXX)
    target_compile_definitions(IEGraphicsModuleOpenGL INTERFACE ILLUMINATION_ENGINE_OPENGL)  # Compile with OpenGL support

    add_dependencies(IEGraphicsModuleOpenGL IEGraphicsModuleOpenGLShaders)  # Add shader pre-compilation as a dependency

    # Add external dependency libraries and include directories to the target
    target_link_libraries(IEGraphicsModuleOpenGL INTERFACE glew glfw glm assimp)
    target_include_directories(IEGraphicsModuleOpenGL INTERFACE ${glew_DIR} ${glfw_DIR} ${glm_DIR} ${stb_image_DIR} ${assimp_DIR})
endif ()
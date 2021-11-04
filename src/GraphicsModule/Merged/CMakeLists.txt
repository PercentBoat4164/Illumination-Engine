set(IEGraphicsModuleMergedSourceFiles
        IeBuffer.hpp
        IeCamera.hpp
        IeCommandPool.hpp
        IeDescriptorSet.hpp
        IeFramebuffer.hpp
        IeImage.hpp
        IePipeline.hpp
        IeRenderEngine.hpp
        IeRenderable.hpp
        IeRenderEngineLink.hpp
        IeRenderPass.hpp
        IeSettings.hpp
        IeShader.hpp
        IeTexture.hpp
        IEGMMergedMain.cpp)
add_executable(IEGraphicsModuleMerged ${IEGraphicsModuleMergedSourceFiles})
set_target_properties(IEGraphicsModuleMerged PROPERTIES LINKER_LANGUAGE CXX)
add_dependencies(IEGraphicsModuleMerged IELogModule)
addAssImp(IEGraphicsModuleMerged)
addGLEW(IEGraphicsModuleMerged)
addGLM(IEGraphicsModuleMerged)
addVulkan(IEGraphicsModuleMerged)
addGLFW(IEGraphicsModuleMerged)
addLog4cplus(IEGraphicsModuleMerged)
target_include_directories(IEGraphicsModuleMerged PUBLIC ${CMAKE_SOURCE_DIR}/src)
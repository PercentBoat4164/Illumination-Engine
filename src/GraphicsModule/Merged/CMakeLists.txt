set(IEGraphicsModuleMergedSourceFiles
        IEBuffer.hpp
        IECamera.hpp
        IECommandPool.hpp
        IEDescriptorSet.hpp
        IEFramebuffer.hpp
        IEImage.hpp
        IEPipeline.hpp
        IERenderEngine.hpp
        IERenderable.hpp
        IERenderEngineLink.hpp
        IERenderPass.hpp
        IESettings.hpp
        IEShader.hpp
        IETexture.hpp
        IEGMMergedMain.cpp IEFramebufferAttachment.hpp IEVertex.hpp IEMesh.hpp IEVersion.hpp)
add_executable(IEGraphicsModuleMerged ${IEGraphicsModuleMergedSourceFiles})
set_target_properties(IEGraphicsModuleMerged PROPERTIES LINKER_LANGUAGE CXX)
add_subdirectory(DefaultShaders)
add_dependencies(IEGraphicsModuleMerged IEGraphicsModuleMergedShaders)
add_dependencies(IEGraphicsModuleMerged IELogModule)
addAssImp(IEGraphicsModuleMerged)
addGLEW(IEGraphicsModuleMerged)
addGLM(IEGraphicsModuleMerged)
addVulkan(IEGraphicsModuleMerged)
addGLFW(IEGraphicsModuleMerged)
addLog4cplus(IEGraphicsModuleMerged)
target_include_directories(IEGraphicsModuleMerged PUBLIC ${ILLUMINATION_ENGINE_BASE_DIRECTORY}/src)
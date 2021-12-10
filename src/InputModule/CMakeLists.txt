set(IEInputModuleSourceFiles
        IeKeyboard.hpp
        IEInputModuleMain.cpp)
add_executable(IEInputModule ${IEInputModuleSourceFiles})
target_include_directories(IEInputModule PUBLIC ${ILLUMINATION_ENGINE_BASE_DIRECTORY}/src)
add_dependencies(IEInputModule IEGraphicsModuleOpenGL)
addAssImp(IEInputModule)
addGLEW(IEInputModule)
addGLFW(IEInputModule)
set(IELogModuleSourceFiles
        IELogger.hpp
        IELogModuleMain.cpp)
add_executable(IELogModule ${IELogModuleSourceFiles})
set_target_properties(IELogModule PROPERTIES LINKER_LANGUAGE CXX)
addSpdlog(IELogModule)
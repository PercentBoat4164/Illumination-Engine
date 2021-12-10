set(IELogModuleSourceFiles
        Log.hpp
        IELogModuleMain.cpp)
add_executable(IELogModule ${IELogModuleSourceFiles})
set_target_properties(IELogModule PROPERTIES LINKER_LANGUAGE CXX)
addLog4cplus(IELogModule)
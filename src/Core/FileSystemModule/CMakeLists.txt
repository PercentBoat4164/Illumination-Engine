set(IEFileSystemModuleSourceFiles
        IEFileSystemModuleMain.cpp IEFileSystem.hpp IEFile.hpp)
add_executable(IEFileSystemModule ${IEFileSystemModuleSourceFiles})
set_target_properties(IEFileSystemModule PROPERTIES LINKER_LANGUAGE CXX)
addSpdlog(IEFileSystemModule)
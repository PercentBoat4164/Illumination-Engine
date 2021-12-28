set(IEFileSystemModuleSourceFiles  # Gather sources
        IEFileSystemModuleMain.cpp
        IEFileSystem.hpp
        IEFile.hpp
        )

# Create and define properties for the executable target
add_executable(IEFileSystemModule ${IEFileSystemModuleSourceFiles})
set_target_properties(IEFileSystemModule PROPERTIES LINKER_LANGUAGE CXX ENABLE_EXPORTS ON)
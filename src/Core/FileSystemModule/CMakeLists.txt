set(IEFileSystemModuleSourceFiles  # Gather sources
        IEDirectory.cpp
        IEFile.cpp
        IEFileSystem.cpp
        )

# Create and define properties for the library
add_library(IEFileSystemModule ${IEFileSystemModuleSourceFiles})
target_link_libraries(IEFileSystemModule PUBLIC INT_src IELogModule)
set_target_properties(IEFileSystemModule PROPERTIES LINKER_LANGUAGE CXX)
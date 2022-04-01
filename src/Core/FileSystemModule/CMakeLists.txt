set(IEFileSystemModuleSourceFiles  # Gather sources
        IEFileSystem.hpp
        IEFile.hpp
        IEDirectory.hpp
        )

# Create and define properties for the library
add_library(IEFileSystemModule ${IEFileSystemModuleSourceFiles})
set_target_properties(IEFileSystemModule PROPERTIES LINKER_LANGUAGE CXX)
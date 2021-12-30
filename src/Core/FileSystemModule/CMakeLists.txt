set(IEFileSystemModuleSourceFiles  # Gather sources
        IEFileSystem.hpp
        IEFile.hpp
        )

# Create and define properties for the executable target
add_library(IEFileSystemModule ${IEFileSystemModuleSourceFiles})
set_target_properties(IEFileSystemModule PROPERTIES LINKER_LANGUAGE CXX)
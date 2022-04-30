set(IEFileSystemModuleSourceFiles  # Gather sources
        IEFileSystem.hpp
        IEFile.hpp
        IEDirectory.hpp
        IETempFile.hpp IETempDirectory.hpp)

# Create and define properties for the executable target
add_library(IEFileSystemModule ${IEFileSystemModuleSourceFiles})
set_target_properties(IEFileSystemModule PROPERTIES LINKER_LANGUAGE CXX)
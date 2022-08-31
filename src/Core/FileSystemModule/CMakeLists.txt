set(IEFileSystemModuleSourceFiles  # Gather sources
        IEDirectory.cpp
        IEDirectory.hpp
        IEFile.cpp
        IEFile.hpp
        IEFileSystem.cpp
        IEFileSystem.hpp
        )

# Create and define properties for the library
add_library(IEFileSystemModule ${IEFileSystemModuleSourceFiles})
target_link_libraries(IEFileSystemModule PUBLIC IELogModule)
set_target_properties(IEFileSystemModule PROPERTIES LINKER_LANGUAGE CXX)
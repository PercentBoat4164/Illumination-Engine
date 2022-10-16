set(IEFileSystemModuleSourceFiles  # Gather sources
        File.cpp
        File.hpp
        FileSystem.cpp
        FileSystem.hpp
        Importer.cpp
        Importer.hpp
        )

# Create and define properties for the library
add_library(IEFileSystemModule ${IEFileSystemModuleSourceFiles})
target_link_libraries(IEFileSystemModule PUBLIC IELogModule assimp)
target_include_directories(IEFileSystemModule PUBLIC ${assimp_DIR})
set_target_properties(IEFileSystemModule PROPERTIES LINKER_LANGUAGE CXX)
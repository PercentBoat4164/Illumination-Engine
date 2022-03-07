# Add all subdirectories
add_subdirectory(AssetModule)
add_subdirectory(FileSystemModule)
add_subdirectory(LogModule)

# Create and define properties for the library
add_library(IECore INTERFACE IEWindowUser.cpp IEWindowUser.hpp)
set_target_properties(IECore PROPERTIES LINKER_LANGUAGE CXX)

# Add internal dependency libraries to the target
target_link_libraries(IECore INTERFACE IEFileSystemModule)
target_link_libraries(IECore INTERFACE IELogModule)
target_link_libraries(IECore INTERFACE IEAssetModule)

# Add internal includes to the target
target_include_directories(IECore INTERFACE ${ILLUMINATION_ENGINE_BASE_DIRECTORY}/src)
# Add all subdirectories
add_subdirectory(LogModule)
add_subdirectory(AssetModule)
add_subdirectory(FileSystemModule)

# Create and define properties for the library
add_library(IECore INTERFACE)
set_target_properties(IECore PROPERTIES LINKER_LANGUAGE CXX)

# Add internal dependency libraries to the target
target_link_libraries(IECore INTERFACE IELogModule)
target_link_libraries(IECore INTERFACE IEAssetModule)
target_link_libraries(IECore INTERFACE IEFileSystemModule)

# Add internal includes to the target
target_include_directories(IECore INTERFACE ${ILLUMINATION_ENGINE_BASE_DIRECTORY}/src)
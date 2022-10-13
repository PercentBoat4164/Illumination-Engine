# Add all subdirectories
add_subdirectory(AssetModule)
add_subdirectory(LogModule)
add_subdirectory(FileSystemModule)
add_subdirectory(ThreadingModule)

# Create and define properties for the library
add_library(IECore MultiDimensionalVector.cpp Core.cpp Engine.cpp)
set_target_properties(IECore PROPERTIES LINKER_LANGUAGE CXX)

# Add internal dependency libraries to the target
target_link_libraries(IECore PUBLIC IEAssetModule)
target_link_libraries(IECore PUBLIC IELogModule)
target_link_libraries(IECore PUBLIC IEFileSystemModule)
target_link_libraries(IECore PUBLIC IEThreadingModule)

# Add internal includes to the target
target_include_directories(IECore PUBLIC ${IE_BASE_DIR}/src)
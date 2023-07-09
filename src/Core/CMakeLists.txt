# Add all subdirectories
add_subdirectory(FileSystemModule)
add_subdirectory(LogModule)
add_subdirectory(AssetModule)
add_subdirectory(EngineModule)
add_subdirectory(ThreadingModule)

add_library(IECore Core.cpp)

# Add internal dependency libraries to the target
target_link_libraries(IECore PUBLIC IEFileSystemModule)
target_link_libraries(IECore PUBLIC IELogModule)
target_link_libraries(IECore PUBLIC IEAssetModule)
target_link_libraries(IECore PUBLIC IEThreadingModule)
target_link_libraries(IECore PUBLIC IEEngineModule)

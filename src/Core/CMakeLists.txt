# Add all subdirectories
add_subdirectory(LogModule)
add_subdirectory(FileSystemModule)

# Create and define properties for the executable target
add_executable(IECore IECoreMain.cpp)
set_target_properties(IECore PROPERTIES LINKER_LANGUAGE CXX ENABLE_EXPORTS ON)

# Add internal dependency libraries to the target
target_link_libraries(IECore PUBLIC IEFileSystemModule)
target_link_libraries(IECore PUBLIC IELogModule)

# Add internal includes to the target
target_include_directories(IECore PUBLIC ${ILLUMINATION_ENGINE_BASE_DIRECTORY}/src)
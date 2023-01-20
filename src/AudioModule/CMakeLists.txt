set(IEAudioModuleSourceFiles  # Gather sources
		AudioEngine.cpp
		Sound.cpp
		)

# Create and define properties for the library
add_library(IEAudioModule ${IEAudioModuleSourceFiles})
target_include_directories(IEAudioModule PUBLIC ${IE_BASE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(IEAudioModule PROPERTIES LINKER_LANGUAGE CXX)

# Add internal dependency libraries to the target
target_link_libraries(IEAudioModule PUBLIC IECore)

# Add external dependency libraries and include directories to the target
target_link_libraries(IEAudioModule PUBLIC INT_src EXT_AssImp EXT_Audio)
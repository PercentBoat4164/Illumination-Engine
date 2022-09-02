cmake_minimum_required(VERSION 3.14 FATAL_ERROR)  # 3.14 is required for FetchContent
project(IlluminationEngine VERSION 0.0.0)

set(CMAKE_CXX_STANDARD 23)

set(BUILD_SHARED_LIBS OFF)
if (WIN32)
	set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif ()

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)  # Uncomment this line if you have no internet connection

set(ILLUMINATION_ENGINE_BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})  # Directory Illumination Engine is being built in
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)  # Directory to put library files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)  # Directory to put runtime files
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)  # Directory to put static libraries

add_subdirectory(ext)  # Generate external dependencies from source
add_subdirectory(src)  # Generate Illumination Engine from source

add_custom_command(OUTPUT ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ILLUMINATION_ENGINE_BASE_DIRECTORY}/res ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res DEPENDS ${ILLUMINATION_ENGINE_BASE_DIRECTORY}/res COMMENT "Copying resource files...")
file(COPY ${ILLUMINATION_ENGINE_BASE_DIRECTORY}/res DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})  # Move resource files

if (CMAKE_BUILD_TYPE STREQUAL Debug)
	add_compile_options("-g")  # Compile with debug information
else ()
	add_compile_options("-O3")  # Compile with level 3 optimizations
endif ()

# Always leave this line commented whenever committing as it will cause issues if the correct software is not installed.
#add_link_options("-fuse-ld=lld")  # Use LLVM's lld linker for speed
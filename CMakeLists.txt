cmake_minimum_required(VERSION 3.11 FATAL_ERROR)  # 3.11 is required for FetchContent
project(IlluminationEngine VERSION 0.0.0)

set(CMAKE_CXX_STANDARD 23)

set(BUILD_SHARED_LIBS OFF)
if (WIN32)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif ()

# Detect compiler and OS
message("Using ${CMAKE_CXX_COMPILER_ID} on ${CMAKE_SYSTEM_NAME}.")
add_compile_definitions("COMPILER=${CMAKE_CXX_COMPILER_ID}")
add_compile_definitions("OS=${CMAKE_SYSTEM_NAME}")


set(IE_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})  # Directory Illumination Engine is being built in
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)  # Directory to put library files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)  # Directory to put runtime files
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)  # Directory to put static libraries

add_custom_target(CopyResources)
add_custom_command(
		TARGET CopyResources
		COMMAND ${CMAKE_COMMAND} -E copy_directory
		${IE_BASE_DIR}/res
		${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/res &
		DEPENDS ${IE_BASE_DIR}/res
		COMMENT "Copying resource files...")

set(IE_PREFER_LOCAL_LIBS ON)

add_subdirectory(ext)  # Generate external dependencies from source
add_subdirectory(src)  # Generate Illumination Engine from source

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
	add_compile_options(-fexceptions -Wno-unused-function)
endif ()
if (${CMAKE_CXX_COMPILER_ID} STREQUAL "IntelLLVM")
	add_compile_options(-Wunused-command-line-argument)
endif ()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_compile_options(-g)  # Compile with debug information
else ()
    add_compile_options(-O3)  # Compile with level 3 optimizations
endif ()

# Use LLVM's lld linker for speed.
add_link_options("-fuse-ld=lld")
add_compile_options("-fuse-ld=lld")
set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "-fuse-ld=lld")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-fuse-ld=lld")

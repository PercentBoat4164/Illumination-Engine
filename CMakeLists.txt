cmake_minimum_required(VERSION 3.14 FATAL_ERROR)  # 3.14 is required for FetchContent
project(IlluminationEngine VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 23)

if (MSVC)
    set(BUILD_SHARED_LIBS ON)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif ()

set(ILLUMINATION_ENGINE_BASE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})  # Directory Illumination Engine is being built in
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)  # Directory to put library files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)  # Directory to put runtime files
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)  # Directory to put static libraries

add_subdirectory(ext)  # Generate external dependencies from source
add_subdirectory(src)  # Generate Illumination Engine from source

file(COPY ${ILLUMINATION_ENGINE_BASE_DIRECTORY}/res DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})  # Move resource files

if (CMAKE_BUILD_TYPE STREQUAL Debug)
    add_compile_options(-g)  # Compile with debug information
else ()
    add_compile_options(-O3)  # Compile with level 3 optimizations
endif ()

# Require LLVM to be installed on Windows machines
# Use LLVM's lld linker for speed
if (WIN32)
    add_link_options("-fuse-ld=C:\\Program Files\\LLVM\\bin\\ld.lld.exe")  # Make this work for any directory (don't hard code.)
else ()
    add_link_options("-fuse-ld=lld")
endif ()
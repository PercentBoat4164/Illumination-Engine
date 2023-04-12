cmake_minimum_required(VERSION 3.14 FATAL_ERROR)  # 3.14 is required for CPM
project(IlluminationEngine VERSION 0.0.0)

set(CMAKE_CXX_STANDARD 23)

set(BUILD_SHARED_LIBS OFF)

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

# Compilation and linking options
if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_definitions("Clang")

    # Added to prevent known errors from clouding the log.
    add_compile_options("-Wno-nullability-completeness")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (WIN32)
        set(BUILD_SHARED_LIBS ON)
    endif ()

    add_compile_definitions("GCC")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_definitions("MSVC")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    add_compile_definitions("AppleClang")

    # Added to prevent known errors from clouding the log.
    add_compile_options("-Wno-nullability-completeness")
elseif ()
    message(WARNING "Unsupported compiler in use: ${CMAKE_CXX_COMPILER_ID}")
endif ()

if (NOT CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options("-Og")
else ()
    add_compile_options("-g")
endif ()

add_subdirectory(ext)  # Generate external dependencies from source
add_subdirectory(src)  # Generate Illumination Engine from source

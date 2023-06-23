cmake_minimum_required(VERSION 3.14 FATAL_ERROR)  # 3.14 is required for CPM
set(ILLUMINATION_ENGINE_VERSION 0.0.0)
project(IlluminationEngine VERSION ${ILLUMINATION_ENGINE_VERSION})

set(CMAKE_CXX_STANDARD 23)

set(BUILD_SHARED_LIBS OFF)

include(cmake/Threading.cmake)
set(BUILD_RES ${CMAKE_SOURCE_DIR}/build-res)

# Detect compiler and OS
if (${CMAKE_CXX_COMPILER_FRONTEND_VARIANT})
    message("Using ${CMAKE_CXX_COMPILER_FRONTEND_VARIANT} ${CMAKE_CXX_COMPILER_ID} v${CMAKE_CXX_COMPILER_VERSION} on ${CMAKE_SYSTEM_NAME}.")
else ()
    message("Using ${CMAKE_CXX_COMPILER_ID} v${CMAKE_CXX_COMPILER_VERSION} on ${CMAKE_SYSTEM_NAME}.")
endif ()
add_compile_definitions(IE_COMPILER="${CMAKE_CXX_COMPILER_ID}")
add_compile_definitions(IE_COMPILER_FRONTEND="${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}")
add_compile_definitions(IE_COMPILER_VERSION="${CMAKE_CXX_COMPILER_VERSION}")
add_compile_definitions(IE_OS="${CMAKE_SYSTEM_NAME}")

add_compile_definitions(IE_MACOS="Darwin")
add_compile_definitions(IE_WINDOWS="Windows")
add_compile_definitions(IE_LINUX="Linux")

if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    add_compile_definitions(IE_OS_IS_MACOS=true)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    add_compile_definitions(IE_OS_IS_WINDOWS=true)
elseif (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    add_compile_definitions(IE_OS_IS_LINUX=true)
else ()
    message(FATAL_ERROR "Unsupported operating system in use: ${CMAKE_SYSTEM_NAME}")
endif ()


set(IE_BASE_DIR ${CMAKE_CURRENT_SOURCE_DIR})  # Directory Illumination Engine is being built in
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)  # Directory to put library files
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)  # Directory to put runtime files
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)  # Directory to put static libraries

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

    # Eliminate the unnecessary warnings.
    string(REGEX REPLACE " /W[3|4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} CACHE STRING "" FORCE)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    add_compile_definitions("AppleClang")

    # Added to prevent known errors from clouding the log.
    add_compile_options("-Wno-nullability-completeness")
elseif ()
    message(WARNING "Unsupported compiler in use: ${CMAKE_CXX_COMPILER_ID}")
endif ()

# Compilation Options for similar interface compilers
if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    add_compile_options("/Zi" "/Od")  # Complete debugging information with no optimization
else ()
    add_compile_options("-g" "-O0")  # Debugging information with no optimization
endif ()

include(cmake/BundleExecutable.cmake)

set(ILLUMINATION_ENGINE_INTERNAL_RESOURCE_DIR ${CMAKE_SOURCE_DIR}/res-internal)
set(ILLUMINATION_ENGINE_EXTERNAL_RESOURCE_DIR ${CMAKE_SOURCE_DIR}/res-external)

add_subdirectory(ext)  # Generate external dependencies from source
add_subdirectory(src)  # Generate Illumination Engine from source

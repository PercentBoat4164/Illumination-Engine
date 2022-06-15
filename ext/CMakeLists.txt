find_package(Vulkan)  # Check for Vulkan
find_package(OpenGL)  # Check for OpenGL

include(FetchContent)  # Requires CMake 3.14

#Get AssImp
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_SAMPLES OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_ZLIB ON)
message("Downloading AssImp master")
FetchContent_Declare(assimp GIT_REPOSITORY https://github.com/assimp/assimp GIT_TAG master GIT_PROGRESS ON)

#Get GLM
message("Downloading GLM 0.9.9.8")
FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm GIT_TAG 0.9.9.8 GIT_PROGRESS ON)

#Get spdlog
message("Downloading spdlog v1.9.2")
FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog GIT_TAG v1.9.2 GIT_PROGRESS ON)

if (OpenGL_FOUND)
    #Get GLFW
    set(GLFW_BUILD_DOCS OFF)
    set(GLFW_BUILD_TESTS OFF)
    set(GLFW_INSTALL OFF)
    set(GLFW_BUILD_EXAMPLES OFF)
    message("Downloading GLFW 3.3.2")
    FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw GIT_TAG 3.3.2 GIT_PROGRESS ON)

    #Get GLEW
    if (BUILD_SHARED_LIBS)
        set(glew-cmake_BUILD_SHARED ON)
        set(glew-cmake_BUILD_STATIC OFF)
    else ()
        set(glew-cmake_BUILD_SHARED OFF)
        set(glew-cmake_BUILD_STATIC ON)
    endif ()
    message("Downloading GLEW glew-2.2.0")
    FetchContent_Declare(glew URL https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.tgz)

    FetchContent_MakeAvailable(glfw glew)  # Generate all external dependencies which require OpenGL

    #Special commands for generating GLEW
    add_subdirectory(${glew_SOURCE_DIR}/build/cmake ${glew_BINARY_DIR})

    set(OpenGL_FOUND ON PARENT_SCOPE)  # Move package variable up to parent scope
endif ()

if (Vulkan_FOUND)
    #Get vk-bootstrap
    message("Downloading vk-bootstrap v0.5")
    FetchContent_Declare(vk-bootstrap GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap GIT_TAG v0.5 GIT_PROGRESS ON)

    #Get VMA
    message("Downloading VMA v2.3.0+vs2017")
    FetchContent_Declare(vma GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator GIT_TAG v2.3.0+vs2017 GIT_PROGRESS ON)

    FetchContent_MakeAvailable(vk-bootstrap vma)  # Generate all external dependencies which require Vulkan

    set(Vulkan_FOUND ON PARENT_SCOPE)  # Move package variable up to parent scope
endif ()

FetchContent_MakeAvailable(assimp glm spdlog)  # Build all libraries
set_target_properties(assimp PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

#Setup all needed includes
set(assimp_DIR ${assimp_SOURCE_DIR}/include PARENT_SCOPE)
set(glfw_DIR ${glfw_SOURCE_DIR}/include PARENT_SCOPE)
set(glew_DIR ${glew_SOURCE_DIR}/include PARENT_SCOPE)
set(glm_DIR ${glm_SOURCE_DIR} PARENT_SCOPE)
set(stb_image_DIR ${assimp_SOURCE_DIR}/contrib/stb PARENT_SCOPE)
if (Vulkan_FOUND)
    set(vk-bootstrap_DIR ${vk-bootstrap_SOURCE_DIR}/src PARENT_SCOPE)
    set(vma_DIR ${vma_SOURCE_DIR}/src PARENT_SCOPE)
endif ()
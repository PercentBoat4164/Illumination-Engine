find_package(Vulkan REQUIRED)  # Check for Vulkan
find_package(OpenGL REQUIRED)  # Check for OpenGL

include(FetchContent)  # Requires CMake 3.14

# Possibly add https://github.com/libvips/libvips as the image importer of choice. (high speed, high compatibility, highly maintained, but does not use CMake)

# Get AssImp
# ASSIMP settings ignored on first buildset(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
message("Downloading AssImp master")
FetchContent_Declare(assimp GIT_REPOSITORY https://github.com/assimp/assimp GIT_TAG master)

# Get GLM
message("Downloading GLM master")
FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm GIT_TAG master)

# Get spdlog
message("Downloading spdlog v1.x")
FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog GIT_TAG v1.x)

set(Vulkan_FOUND ON PARENT_SCOPE)  # Move package variable up to parent scope
set(Vulkan_LIBRARIES ${Vulkan_LIBRARIES} PARENT_SCOPE)  # Move package variable up to parent scope
set(Vulkan_INCLUDE_DIR ${Vulkan_INCLUDE_DIR} PARENT_SCOPE)  # Move package variable up to parent scope

# Get GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
message("Downloading GLFW master")
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw GIT_TAG master)

# Get GLEW
if (BUILD_SHARED_LIBS)
	set(glew-cmake_BUILD_SHARED ON CACHE BOOL "" FORCE)
	set(glew-cmake_BUILD_STATIC OFF CACHE BOOL "" FORCE)
else ()
	set(glew-cmake_BUILD_SHARED OFF CACHE BOOL "" FORCE)
	set(glew-cmake_BUILD_STATIC ON CACHE BOOL "" FORCE)
endif ()
message("Downloading GLEW glew-2.2.0")
FetchContent_Declare(glew URL https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.tgz SOURCE_SUBDIR build/cmake)

FetchContent_MakeAvailable(glfw glew)  # Generate all external dependencies which require OpenGL

set(OpenGL_FOUND ON PARENT_SCOPE)  # Move package variable up to parent scope

# Get vk-bootstrap
message("Downloading vk-bootstrap from master")
FetchContent_Declare(vk-bootstrap GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap GIT_TAG master)

# Get VMA from master because latest release was from 2017. Master is generally considered stable.
message("Downloading VMA from master")
FetchContent_Declare(vma GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator GIT_TAG master)

FetchContent_MakeAvailable(vk-bootstrap vma)  # Generate all external dependencies which require Vulkan

set(Vulkan_FOUND ON PARENT_SCOPE)  # Move package variable up to parent scope
set(Vulkan_LIBRARIES ${Vulkan_LIBRARIES} PARENT_SCOPE)  # Move package variable up to parent scope
set(Vulkan_INCLUDE_DIRS ${Vulkan_INCLUDE_DIRS} PARENT_SCOPE)  # Move package variable up to parent scope

FetchContent_MakeAvailable(assimp glm spdlog)  # Build all libraries
set_target_properties(assimp PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Setup all needed includes
set(assimp_DIR ${assimp_SOURCE_DIR} PARENT_SCOPE)
set(spdlog_DIR ${spdlog_SOURCE_DIR} PARENT_SCOPE)
set(glfw_DIR ${glfw_SOURCE_DIR} PARENT_SCOPE)
set(glew_DIR ${glew_SOURCE_DIR} PARENT_SCOPE)
set(glm_DIR ${glm_SOURCE_DIR} PARENT_SCOPE)
set(stb_image_DIR ${assimp_SOURCE_DIR}/contrib/stb PARENT_SCOPE)
set(vk-bootstrap_DIR ${vk-bootstrap_SOURCE_DIR}/src PARENT_SCOPE)
set(vma_DIR ${vma_SOURCE_DIR} PARENT_SCOPE)
set(vulkan_DIR ${_vulkanheaders_SOURCE_DIR} PARENT_SCOPE)

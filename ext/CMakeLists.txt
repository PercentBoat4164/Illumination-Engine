find_package(Vulkan)  # Check for Vulkan
find_package(OpenGL)  # Check for OpenGL

include(FetchContent)  # Requires CMake 3.14

# Possibly add https://github.com/libvips/libvips as the image importer of choice. (high speed, high compatibility, highly maintained, but does not use CMake)

# Get AssImp
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_SAMPLES OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_ZLIB ON)
message("Downloading AssImp v5.2.2")
FetchContent_Declare(assimp GIT_REPOSITORY https://github.com/assimp/assimp GIT_TAG v5.2.2)

# Get GLM
message("Downloading GLM 0.9.9.8")
FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm GIT_TAG 0.9.9.8)

# Get spdlog
message("Downloading spdlog  v1.9.2")
FetchContent_Declare(spdlog GIT_REPOSITORY https://github.com/gabime/spdlog GIT_TAG v1.9.2)

if (OpenGL_FOUND)
	# Get GLFW
	set(GLFW_BUILD_DOCS OFF)
	set(GLFW_BUILD_TESTS OFF)
	set(GLFW_INSTALL OFF)
	set(GLFW_BUILD_EXAMPLES OFF)
	message("Downloading GLFW  3.3.6")
	FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw GIT_TAG 3.3.6)

	# Get GLEW
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

	# Special commands for generating GLEW
	add_subdirectory(${glew_SOURCE_DIR}/build/cmake ${glew_BINARY_DIR})

	set(OpenGL_FOUND ON PARENT_SCOPE)  # Move package variable up to parent scope
endif ()

if (NOT Vulkan_FOUND)
	# Get Vulkan SDK
	# The Vulkan SDK is made of many different repositories, but they will be treated as one library.
	message("Downloading the latest Vulkan SDK")
	FetchContent_Declare(_vulkanHeaders GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers GIT_TAG master)
	FetchContent_Declare(_vulkanLoader GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Loader GIT_TAG master)
	FetchContent_Declare(_vulkanValidationLayers GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-ValidationLayers GIT_TAG master)
	FetchContent_Declare(_vulkanKhronosTools GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Tools GIT_TAG master)
	FetchContent_Declare(_vulkanLunarGTools GIT_REPOSITORY https://github.com/LunarG/VulkanTools GIT_TAG master)
	FetchContent_Declare(_vulkanGlslang GIT_REPOSITORY https://github.com/KhronosGroup/glslang GIT_TAG master)
	FetchContent_Declare(_vulkanShaderc GIT_REPOSITORY https://github.com/google/shaderc GIT_TAG master)
	FetchContent_Declare(_vulkanSPIRVHeaders GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers GIT_TAG master)
	FetchContent_Declare(_vulkanSPIRVTools GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools GIT_TAG master)
	FetchContent_Declare(_vulkanSPIRVCross GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross GIT_TAG master)
	FetchContent_Declare(_vulkanGfxReconstruct GIT_REPOSITORY https://github.com/LunarG/gfxreconstruct GIT_TAG master)
	FetchContent_Declare(_vulkanSPIRVReflect GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Reflect GIT_TAG master)
	FetchContent_Declare(_vulkanExtensionLayer GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-ExtensionLayer GIT_TAG master)
	FetchContent_Declare(_vulkanDXC GIT_REPOSITORY https://github.com/microsoft/DirectXShaderCompiler GIT_TAG master)

	# Build all of the repositories
	FetchContent_MakeAvailable(_vulkanHeaders _vulkanLoader _vulkanValidationLayers _vulkanKhronosTools _vulkanLunarGTools _vulkanGlslang _vulkanShaderc _vulkanSPIRVHeaders _vulkanSPIRVTools _vulkanSPIRVCross _vulkanGfxReconstruct _vulkanSPIRVReflect _vulkanExtensionLayer _vulkanDXC)

	# Merge them into one library
	add_library(vulkan PUBLIC _vulkanHeaders _vulkanLoader _vulkanValidationLayers _vulkanKhronosTools _vulkanLunarGTools _vulkanGlslang _vulkanShaderc _vulkanSPIRVHeaders _vulkanSPIRVTools _vulkanSPIRVCross _vulkanGfxReconstruct _vulkanSPIRVReflect _vulkanExtensionLayer _vulkanDXC)

	# Setup the CMake variables required for Vulkan
	set(Vulkan_FOUND ON PARENT_SCOPE)  # Move package variable up to parent scope
endif ()

if (Vulkan_FOUND)
	# Get vk-bootstrap
	message("Downloading vk-bootstrap v0.5")
	FetchContent_Declare(vk-bootstrap GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap GIT_TAG v0.5)

	# Get VMA from master because latest release was from 2017. Master is generally considered stable.
	message("Downloading VMA from master")
	FetchContent_Declare(vma GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator GIT_TAG master)

	FetchContent_MakeAvailable(vk-bootstrap vma)  # Generate all external dependencies which require Vulkan

	set(Vulkan_FOUND ON PARENT_SCOPE)  # Move package variable up to parent scope
endif ()

FetchContent_MakeAvailable(assimp glm spdlog)  # Build all libraries
set_target_properties(assimp PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Setup all needed includes
set(assimp_DIR ${assimp_SOURCE_DIR}/include PARENT_SCOPE)
set(glfw_DIR ${glfw_SOURCE_DIR}/include PARENT_SCOPE)
set(glew_DIR ${glew_SOURCE_DIR}/include PARENT_SCOPE)
set(glm_DIR ${glm_SOURCE_DIR} PARENT_SCOPE)
set(stb_image_DIR ${assimp_SOURCE_DIR}/contrib/stb PARENT_SCOPE)
if (Vulkan_FOUND)
	set(vk-bootstrap_DIR ${vk-bootstrap_SOURCE_DIR}/src PARENT_SCOPE)
	set(vma_DIR ${vma_SOURCE_DIR}/include PARENT_SCOPE)
endif ()
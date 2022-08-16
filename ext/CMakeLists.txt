find_package(Vulkan)  # Check for Vulkan
find_package(OpenGL)  # Check for OpenGL

include(FetchContent)  # Requires CMake 3.14

# Possibly add https://github.com/libvips/libvips as the image importer of choice. (high speed, high compatibility, highly maintained, but does not use CMake)

# Get AssImp
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
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

# Get Vulkan SDK if it was not found before
if (NOT Vulkan_FOUND)
	# The Vulkan SDK is made of many different repositories, but they will be treated as one library.
	message("Downloading the latest Vulkan SDK")
	message("Downloading Vulkan Headers")
	FetchContent_Declare(_vulkanHeaders GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers GIT_TAG main)
	FetchContent_MakeAvailable(_vulkanHeaders)
	set(Vulkan_INCLUDE_DIR ${_vulkanheaders_SOURCE_DIR}/include CACHE PATH "" FORCE)

	message("Downloading Vulkan Loader")
	FetchContent_Declare(_vulkanLoader GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Loader GIT_TAG master)
	FetchContent_MakeAvailable(_vulkanLoader)

	message("Downloading Vulkan SPIR-V Headers")
	FetchContent_Declare(_vulkanSPIRVHeaders GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers GIT_TAG master)  # Must be gotten before _vulkanSPIRVTools
	FetchContent_MakeAvailable(_vulkanSPIRVHeaders)

	message("Downloading Vulkan Headers")
	# todo Add Robin Hood hashing before _vulkanValidationLayers. Optionally?
	set(USE_ROBIN_HOOD_HASHING OFF CACHE BOOL "use robin hood hashing" FORCE)  # Skip robin hood hashing
	FetchContent_Declare(_vulkanSPIRVTools GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools GIT_TAG master)  # Must be gotten before _vulkanValidationLayers
	FetchContent_MakeAvailable(_vulkanSPIRVTools)

	message("Downloading Vulkan Validation Layers")
	FetchContent_Declare(_vulkanValidationLayers GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-ValidationLayers GIT_TAG master)  # Must be loaded before _vulkanKhronosTools
	FetchContent_MakeAvailable(_vulkanValidationLayers)

	message("Downloading Vulkan Khronos Tools")
	set(VulkanHeaders_INCLUDE_DIR ${_vulkanheaders_SOURCE_DIR}/include CACHE PATH "" FORCE)
	set(VulkanRegistry_DIR ${_vulkanheaders_SOURCE_DIR}/registry CACHE PATH "" FORCE)
	set(VULKAN_VALIDATIONLAYERS_INSTALL_DIR ${_vulkanvalidationlayers_SOURCE_DIR} CACHE PATH "" FORCE)
	FetchContent_Declare(_vulkanKhronosTools GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Tools GIT_TAG master)
	FetchContent_MakeAvailable(_vulkanKhronosTools)

	message("Downloading Vulkan LunarG Tools")
	set(VULKAN_HEADERS_INSTALL_DIR ${_vulkanheaders_SOURCE_DIR})
	set(VULKAN_LOADER_INSTALL_DIR ${_vulkanloader_SOURCE_DIR})
	FetchContent_Declare(_vulkanLunarGTools GIT_REPOSITORY https://github.com/LunarG/VulkanTools GIT_TAG master)
	FetchContent_MakeAvailable(_vulkanLunarGTools)

	message("Downloading GLSLang")
	FetchContent_Declare(_vulkanGlslang GIT_REPOSITORY https://github.com/KhronosGroup/glslang GIT_TAG master)
	FetchContent_MakeAvailable(_vulkanGlslang)

	message("Downloading ShaderC")
	FetchContent_Declare(_vulkanShaderc GIT_REPOSITORY https://github.com/google/shaderc GIT_TAG master)
	FetchContent_MakeAvailable(_vulkanShaderc)

	message("Downloading SPIR-V Cross")
	FetchContent_Declare(_vulkanSPIRVCross GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross GIT_TAG master)
	FetchContent_MakeAvailable(_vulkanSPIRVCross)

	message("Downloading GFXReconstruct")
	FetchContent_Declare(_vulkanGfxReconstruct GIT_REPOSITORY https://github.com/LunarG/gfxreconstruct GIT_TAG master)
	FetchContent_MakeAvailable(_vulkanGfxReconstruct)

	message("Downloading SPIR-V Reflect")
	FetchContent_Declare(_vulkanSPIRVReflect GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Reflect GIT_TAG master)
	FetchContent_MakeAvailable(_vulkanSPIRVReflect)

	message("Downloading Vulkan Extension Layer")
	FetchContent_Declare(_vulkanExtensionLayer GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-ExtensionLayer GIT_TAG master)
	FetchContent_MakeAvailable(_vulkanExtensionLayer)

	message("Downloading DirectX Shader Compiler")
	FetchContent_Declare(_vulkanDXC GIT_REPOSITORY https://github.com/microsoft/DirectXShaderCompiler GIT_TAG master)
	FetchContent_MakeAvailable(_vulkanDXC)

	# Merge them into one library
	add_library(vulkan PUBLIC _vulkanHeaders _vulkanLoader _vulkanValidationLayers _vulkanKhronosTools _vulkanLunarGTools _vulkanGlslang _vulkanShaderc _vulkanSPIRVHeaders _vulkanSPIRVTools _vulkanSPIRVCross _vulkanGfxReconstruct _vulkanSPIRVReflect _vulkanExtensionLayer _vulkanDXC)

	set(Vulkan_FOUND ON PARENT_SCOPE)  # Move package variable up to parent scope
	set(Vulkan_LIBRARIES ${Vulkan_LIBRARIES} PARENT_SCOPE)  # Move package variable up to parent scope
	set(Vulkan_INCLUDE_DIR ${Vulkan_INCLUDE_DIR} PARENT_SCOPE)  # Move package variable up to parent scope
endif ()

if (OpenGL_FOUND)
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
	FetchContent_Declare(glew URL https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.tgz)

	FetchContent_MakeAvailable(glfw glew)  # Generate all external dependencies which require OpenGL

	# Special commands for generating GLEW
	add_subdirectory(${glew_SOURCE_DIR}/build/cmake ${glew_BINARY_DIR})

	set(OpenGL_FOUND ON PARENT_SCOPE)  # Move package variable up to parent scope
endif ()

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
set(glfw_DIR ${glfw_SOURCE_DIR} PARENT_SCOPE)
set(glew_DIR ${glew_SOURCE_DIR} PARENT_SCOPE)
set(glm_DIR ${glm_SOURCE_DIR} PARENT_SCOPE)
set(stb_image_DIR ${assimp_SOURCE_DIR}/contrib/stb PARENT_SCOPE)
if (Vulkan_FOUND)
	set(vk-bootstrap_DIR ${vk-bootstrap_SOURCE_DIR}/src PARENT_SCOPE)
	set(vma_DIR ${vma_SOURCE_DIR} PARENT_SCOPE)
	set(vulkan_DIR ${_vulkanheaders_SOURCE_DIR} PARENT_SCOPE)
endif ()
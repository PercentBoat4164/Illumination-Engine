# @todo Make this easier to maintain with the use of for( loops.
# @todo Select individual versions for each library.
# @todo Populate all options available in each library.
# @todo Allow usage of local libraries instead of remotes. (Already implemented?)

include(FetchContent)  # Requires CMake 3.14

set(IE_DEPS_DOWNLOAD_DIR   ${CMAKE_CURRENT_SOURCE_DIR}/deps)
set(IE_DEPS_BUILD_DIR      ${CMAKE_CURRENT_BINARY_DIR}/deps)
set(IE_DOWNLOAD_GUARD_FILE ${CMAKE_SOURCE_DIR}/__DOWNLOAD_GUARD__)

set(FETCHCONTENT_BASE_DIR ${IE_DEPS_DOWNLOAD_DIR})

# Possibly add https://github.com/libvips/libvips as the image importer of choice. (high speed, high compatibility, highly maintained, but does not use CMake)



# All options for Dependencies should go here
# AssImp Options
set(ASSIMP_HUNTER_ENABLED OFF)
set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
if (MACOS)
	set(ASSIMP_BUILD_FRAMEWORK OFF)
else ()
	set(ASSIMP_BUILD_FRAMEWORK OFF)
endif ()
set(ASSIMP_DOUBLE_PRECISION OFF)
set(ASSIMP_OPT_BUILD_PACKAGES OFF)
set(ASSIMP_ANDROID_JNIIOSYSTEM OFF)
set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_BUILD_ZLIB ON)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT ON)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_SAMPLES OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_COVERALLS OFF)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_WARNINGS_AS_ERRORS ON)
set(ASSIMP_ASAN N)
set(ASSIMP_UBSAN ON)
set(ASSIMP_BUILD_DOCS OFF)  # Obsolete
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF)
set(ASSIMP_IGNORE_GIT_HASH ON)
set(ASSIMP_INSTALL_PDB OFF)
set(USE_STATIC_CRT OFF)
set(ASSIMP_BUILD_DRACO OFF)  # Fails as of AssImp v5.2.5
if (WINDOWS)
	set(ASSIMP_BUILD_ASSIMP_VIEW OFF)
else()
	set(ASSIMP_BUILD_ASSIMP_VIEW OFF)
endif()

# GLEW Options
set(GLEW_VERBOSE OFF)
set(GLEW_USE_STATIC_LIBS NOT ${BUILD_SHARED_LIBS} FORCE)

# GLFW Options
set(GLFW_BUILD_DOCS OFF FORCE)
set(GLFW_BUILD_TESTS OFF FORCE)
set(GLFW_INSTALL OFF FORCE)
set(GLFW_BUILD_EXAMPLES OFF FORCE)



# Library versions
set(assimp_VERSION v5.2.5)
set(assimp_VERSION_FILE ${IE_DEPS_DOWNLOAD_DIR}/assimp_version.txt)

set(glm_VERSION master)
set(glm_VERSION_FILE ${IE_DEPS_DOWNLOAD_DIR}/glm_version.txt)

set(glew_VERSION https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.tgz)
set(glew_VERSION_FILE ${IE_DEPS_DOWNLOAD_DIR}/glew_version.txt)

set(glfw_VERSION master)
set(glfw_VERSION_FILE ${IE_DEPS_DOWNLOAD_DIR}/glfw_version.txt)

set(spdlog_VERSION v1.x)
set(spdlog_VERSION_FILE ${IE_DEPS_DOWNLOAD_DIR}/spdlog_version.txt)

set(vk-bootstrap_VERSION master)
set(vk-bootstrap_VERSION_FILE ${IE_DEPS_DOWNLOAD_DIR}/vk-bootstrap_version.txt)

set(vma_VERSION master)
set(vma_VERSION_FILE ${IE_DEPS_DOWNLOAD_DIR}/vma_version.txt)

# Download arguments
set(assimp_DOWNLOAD_ARGS       GIT_REPOSITORY https://github.com/assimp/assimp                                  GIT_TAG       ${assimp_VERSION})
set(glm_DOWNLOAD_ARGS          GIT_REPOSITORY https://github.com/g-truc/glm                                     GIT_TAG       ${glm_VERSION})
set(glew_DOWNLOAD_ARGS         URL            ${glew_VERSION}                                                   SOURCE_SUBDIR build/cmake)
set(glfw_DOWNLOAD_ARGS         GIT_REPOSITORY https://github.com/glfw/glfw                                      GIT_TAG       ${glfw_VERSION})
set(spdlog_DOWNLOAD_ARGS       GIT_REPOSITORY https://github.com/gabime/spdlog                                  GIT_TAG       ${spdlog_VERSION})
set(vk-bootstrap_DOWNLOAD_ARGS GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap                    GIT_TAG       ${vk-bootstrap_VERSION})
set(vma_DOWNLOAD_ARGS          GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator GIT_TAG       ${vma_VERSION})



# FetchContent_Declare should go before the Download Guard
FetchContent_Declare(assimp       ${assimp_DOWNLOAD_ARGS}       FIND_PACKAGE_ARGS NAMES AssImp)
FetchContent_Declare(glm          ${glm_DOWNLOAD_ARGS}          FIND_PACKAGE_ARGS NAMES GLM)
FetchContent_Declare(glew         ${glew_DOWNLOAD_ARGS}         FIND_PACKAGE_ARGS NAMES GLEW)
FetchContent_Declare(glfw         ${glfw_DOWNLOAD_ARGS}         FIND_PACKAGE_ARGS NAMES GLFW)
FetchContent_Declare(spdlog       ${spdlog_DOWNLOAD_ARGS}       FIND_PACKAGE_ARGS NAMES spdlog)
FetchContent_Declare(vk-bootstrap ${vk-bootstrap_DOWNLOAD_ARGS} FIND_PACKAGE_ARGS NAMES vk-bootstrap)
FetchContent_Declare(vma          ${vma_DOWNLOAD_ARGS}          FIND_PACKAGE_ARGS NAMES vma)

# Lock the Download Guard
file(LOCK ${IE_DOWNLOAD_GUARD_FILE} RESULT_VARIABLE HAS_DOWNLOAD_GUARD_LOCK TIMEOUT 0)
if (NOT ${HAS_DOWNLOAD_GUARD_LOCK} MATCHES 0)
	# Wait until the Download Guard is unlocked
	file(LOCK ${IE_DOWNLOAD_GUARD_FILE})
	file(REMOVE ${IE_DOWNLOAD_GUARD_FILE})
else ()
	# Do all the downloads here. They will only run in a single CMake instance no matter how many are running
	file(APPEND ${assimp_VERSION_FILE})
	file(APPEND ${glm_VERSION_FILE})
	file(APPEND ${glew_VERSION_FILE})
	file(APPEND ${glfw_VERSION_FILE})
	file(APPEND ${spdlog_VERSION_FILE})
	file(APPEND ${vk-bootstrap_VERSION_FILE})
	file(APPEND ${vma_VERSION_FILE})

	file(READ ${assimp_VERSION_FILE}       assimp_OLD_VERSION)
	file(READ ${glm_VERSION_FILE}          glm_OLD_VERSION)
	file(READ ${glew_VERSION_FILE}         glew_OLD_VERSION)
	file(READ ${glfw_VERSION_FILE}         glfw_OLD_VERSION)
	file(READ ${spdlog_VERSION_FILE}       spdlog_OLD_VERSION)
	file(READ ${vk-bootstrap_VERSION_FILE} vk-bootstrap_OLD_VERSION)
	file(READ ${vma_VERSION_FILE}          vma_OLD_VERSION)

	# Should only include FetchContent_Populate
	if (NOT assimp_OLD_VERSION STREQUAL assimp_VERSION)
		FetchContent_Populate(assimp       SUBBUILD_DIR ${IE_DEPS_BUILD_DIR}/assimp_build		  SOURCE_DIR ${IE_DEPS_DOWNLOAD_DIR}/assimp_src       BINARY_DIR ${IE_DEPS_BUILD_DIR}/assimp_bin       ${assimp_DOWNLOAD_ARGS})
	endif ()
	if (NOT glm_OLD_VERSION STREQUAL glm_VERSION)
		FetchContent_Populate(glm          SUBBUILD_DIR ${IE_DEPS_BUILD_DIR}/glm_build		  SOURCE_DIR ${IE_DEPS_DOWNLOAD_DIR}/glm_src          BINARY_DIR ${IE_DEPS_BUILD_DIR}/glm_bin          ${glm_DOWNLOAD_ARGS})
	endif ()
	if (NOT glew_OLD_VERSION STREQUAL glew_VERSION)
		FetchContent_Populate(glew         SUBBUILD_DIR ${IE_DEPS_BUILD_DIR}/glew_build		  SOURCE_DIR ${IE_DEPS_DOWNLOAD_DIR}/glew_src         BINARY_DIR ${IE_DEPS_BUILD_DIR}/glew_bin         ${glew_DOWNLOAD_ARGS})
	endif ()
	if (NOT glfw_OLD_VERSION STREQUAL glfw_VERSION)
		FetchContent_Populate(glfw         SUBBUILD_DIR ${IE_DEPS_BUILD_DIR}/glfw_build		  SOURCE_DIR ${IE_DEPS_DOWNLOAD_DIR}/glfw_src         BINARY_DIR ${IE_DEPS_BUILD_DIR}/glfw_bin         ${glfw_DOWNLOAD_ARGS})
	endif ()
	if (NOT spdlog_OLD_VERSION STREQUAL spdlog_VERSION)
		FetchContent_Populate(spdlog       SUBBUILD_DIR ${IE_DEPS_BUILD_DIR}/spdlog_build		  SOURCE_DIR ${IE_DEPS_DOWNLOAD_DIR}/spdlog_src       BINARY_DIR ${IE_DEPS_BUILD_DIR}/spdlog_bin       ${spdlog_DOWNLOAD_ARGS})
	endif ()
	if (NOT vk-bootstrap_OLD_VERSION STREQUAL vk-bootstrap_VERSION)
		FetchContent_Populate(vk-bootstrap SUBBUILD_DIR ${IE_DEPS_BUILD_DIR}/vk-bootstrap_build SOURCE_DIR ${IE_DEPS_DOWNLOAD_DIR}/vk-bootstrap_src BINARY_DIR ${IE_DEPS_BUILD_DIR}/vk-bootstrap_bin ${vk-bootstrap_DOWNLOAD_ARGS})
	endif ()
	if (NOT vma_OLD_VERSION STREQUAL vma_VERSION)
		FetchContent_Populate(vma          SUBBUILD_DIR ${IE_DEPS_BUILD_DIR}/vma_build		  SOURCE_DIR ${IE_DEPS_DOWNLOAD_DIR}/vma_src          BINARY_DIR ${IE_DEPS_BUILD_DIR}/vma_bin          ${vma_DOWNLOAD_ARGS})
	endif ()

	# Unlock the Download Guard
	file(REMOVE ${IE_DOWNLOAD_GUARD_FILE})

	# Update version information
	file(WRITE ${assimp_VERSION_FILE}       ${assimp_VERSION})
	file(WRITE ${glm_VERSION_FILE}          ${glm_VERSION})
	file(WRITE ${glew_VERSION_FILE}         ${glew_VERSION})
	file(WRITE ${glfw_VERSION_FILE}         ${glfw_VERSION})
	file(WRITE ${spdlog_VERSION_FILE}       ${spdlog_VERSION})
	file(WRITE ${vk-bootstrap_VERSION_FILE} ${vk-bootstrap_VERSION})
	file(WRITE ${vma_VERSION_FILE}          ${vma_VERSION})
endif ()

# FetchContent_MakeAvailable should go after the Download Guard
FetchContent_MakeAvailable(assimp)
FetchContent_MakeAvailable(glew)
FetchContent_MakeAvailable(glfw)
FetchContent_MakeAvailable(glm)
FetchContent_MakeAvailable(spdlog)
FetchContent_MakeAvailable(vk-bootstrap)
FetchContent_MakeAvailable(vma)


# find_package should go after FetchContent_MakeAvailable
find_package(Vulkan COMPONENTS glslc REQUIRED)  # Check for Vulkan and GLSLC
find_package(OpenGL REQUIRED)  # Check for OpenGL
find_package(AssImp REQUIRED)  # Check for AssImp
find_package(GLM REQUIRED)  # Check for GLM
find_package(GLEW REQUIRED)  # Check for GLEW
find_package(GLFW REQUIRED)  # Check for GLFW
find_package(spdlog REQUIRED)  # Check for spdlog
find_package(vk-bootstrap REQUIRED)  # Check for vk-bootstrap
find_package(vma REQUIRED)  # Check for vma

add_library(Vulkan INTERFACE)
target_link_libraries(Vulkan INTERFACE Vulkan::Vulkan)
add_library(OpenGL INTERFACE)
target_link_libraries(OpenGL INTERFACE OpenGL::GL OpenGL::GLU)
add_library(AssImp INTERFACE)
target_link_libraries(AssImp INTERFACE assimp)
add_library(GLEW INTERFACE)
target_link_libraries(GLEW INTERFACE GLEW::GLEW)
add_library(GLFW INTERFACE)
target_link_libraries(GLFW INTERFACE GLFW::GLFW)
add_library(GLM INTERFACE)
target_link_libraries(GLM INTERFACE glm::glm)
add_library(SpdLog INTERFACE)
target_link_libraries(SpdLog INTERFACE spdlog)
add_library(VkBootstrap INTERFACE)
target_link_libraries(VkBootstrap INTERFACE VkBootstrap)
add_library(VMA INTERFACE)
target_link_libraries(VMA INTERFACE vma)
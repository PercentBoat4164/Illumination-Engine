include(FetchContent)  # Requires CMake 3.11
include(FindPkgConfig)

set(IE_DEPS_DOWNLOAD_DIR ${IE_BASE_DIR}/ext/deps)
set(IE_DEPS_BUILD_DIR ${CMAKE_BINARY_DIR}/deps)
set(IE_DOWNLOAD_GUARD_FILE ${CMAKE_SOURCE_DIR}/__DOWNLOAD_GUARD__)

set(FETCHCONTENT_BASE_DIR ${IE_DEPS_DOWNLOAD_DIR})

# Possibly add https://github.com/libvips/libvips as the image importer of choice. (high speed, high compatibility, highly maintained, but does not use CMake)

# dep_DATA =         NAME        MODULE_NAME  MIN_VERSION MAX_VERSION EXPORT_LIB BUILD_DIR INCLUDE_DIR DOWNLOAD_ARGS
# GitHub hosted dependencies
set(assimp_DATA assimp assimp 5.2.5 5.2.5 EXT_AssImp "" "/include" "GIT_REPOSITORY https://github.com/assimp/assimp GIT_PROGRESS ON GIT_TAG v5.2.5")
set(glm_DATA glm glm "NULL" "NULL" EXT_GLM "" "" "GIT_REPOSITORY https://github.com/g-truc/glm GIT_PROGRESS ON GIT_TAG master")
set(glfw_DATA glfw glfw3 3.3.6 3.3.8 EXT_GLFW "" "/include" "GIT_REPOSITORY https://github.com/glfw/glfw GIT_PROGRESS ON GIT_TAG 3.3.8")
set(spdlog_DATA spdlog spdlog 1.10.0 1.10.0 EXT_SpdLog "" "/include" "GIT_REPOSITORY https://github.com/gabime/spdlog GIT_PROGRESS ON GIT_TAG v1.x")
set(VkBootstrap_DATA VkBootstrap vk-bootstrap 0.6 0.6 EXT_Vulkan "" "/src" "GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap GIT_PROGRESS ON GIT_TAG v0.6")
set(vma_DATA vma vma 3.0.1 3.0.1 EXT_Vulkan "" "/include" "GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator GIT_PROGRESS ON GIT_TAG v3.0.1")
set(GITHUB assimp_DATA glm_DATA glfw_DATA spdlog_DATA VkBootstrap_DATA vma_DATA)
set(GITHUB_NAME 0)
set(GITHUB_MODULE 1)
set(GITHUB_MIN_VERSION 2)
set(GITHUB_MAX_VERSION 3)
set(GITHUB_EXPORT_LIB 4)
set(GITHUB_BUILD_DIR 5)
set(GITHUB_INCLUDE_DIR 6)
set(GITHUB_DOWNLOAD_ARGS 7)
set(GITHUB_REQUESTED_VERSION -1)

# dep_DATA =  NAME MODULE_NAME MIN_VERSION MAX_VERSION EXPORT_LIB BUILD_DIR      INCLUDE_DIR DOWNLOAD_ARGS
# URL fetch dependencies
set(glew_DATA glew glew 2.2.0 2.2.0 EXT_OpenGL "/build/cmake" "/include" "URL https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.tgz")
set(URL glew_DATA)
set(URL_NAME 0)
set(URL_MODULE 1)
set(URL_MIN_VERSION 2)
set(URL_MAX_VERSION 3)
set(URL_EXPORT_LIB 4)
set(URL_BUILD_DIR 5)
set(URL_INCLUDE_DIR 6)
set(URL_DOWNLOAD_ARGS 7)
set(URL_URL -1)

# dep_DATA =    NAME   MODULE_NAME    REQUIRED_COMPONENTS            MIN_VERSION MAX_VERSION EXPORT_LIB
# Local dependencies
set(Vulkan_DATA Vulkan Vulkan::Vulkan "shaderc_combined SPIRV-Tools" 1.0.0 1.3.231 EXT_Vulkan)
set(OpenGL_DATA OpenGL OpenGL "" 0.0 0.0 EXT_OpenGL)
set(LOCAL Vulkan_DATA OpenGL_DATA)
set(LOCAL_NAME 0)
set(LOCAL_MODULE 1)
set(LOCAL_REQUIRED_COMPONENTS 2)
set(LOCAL_MIN_VERSION 3)
set(LOCAL_MAX_VERSION 4)
set(LOCAL_EXPORT_LIB 5)


# All options for Dependencies should go here
# AssImp Options
set(ASSIMP_HUNTER_ENABLED OFF)
set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS})
if (MACOS)
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
set(ASSIMP_ASAN OFF)
set(ASSIMP_UBSAN OFF)
set(ASSIMP_BUILD_DOCS OFF)  # Obsolete
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF)
set(ASSIMP_IGNORE_GIT_HASH ON)
set(ASSIMP_INSTALL_PDB OFF)
set(USE_STATIC_CRT OFF)
set(ASSIMP_BUILD_DRACO OFF)  # Fails as of AssImp v5.2.5
if (WINDOWS)
	set(ASSIMP_BUILD_ASSIMP_VIEW OFF)
endif ()

# GLM Options
# Uncomment options that should be enabled.
#add_compile_definitions(GLM_FORCE_MESSAGES)
#add_compile_definitions(GLM_FORCE_COMPILER_UNKNOWN)
#add_compile_definitions(GLM_FORCE_ARCH_UNKNOWN)
#add_compile_definitions(GLM_FORCE_CXX_UNKNOWN)
#add_compile_definitions(GLM_FORCE_CXX)
#add_compile_definitions(GLM_FORCE_EXPLICIT_CTOR)
#add_compile_definitions(GLM_FORCE_INLINE)
#add_compile_definitions(GLM_FORCE_ALIGNED_GENTYPES)
#add_compile_definitions(GLM_FORCE_DEFAULT_ALIGNED_GENTYPES)
#add_compile_definitions(GLM_FORCE_INTRINSICS)
#add_compile_definitions(GLM_FORCE_PRECISION_LOWP_FLOAT)
#add_compile_definitions(GLM_FORCE_PRECISION_MEDIUMP_FLOAT)
#add_compile_definitions(GLM_FORCE_PRECISION_HIGHP_FLOAT)
#add_compile_definitions(GLM_FORCE_PRECISION_LOWP_DOUBLE)
#add_compile_definitions(GLM_FORCE_PRECISION_MEDIUMP_DOUBLE)
#add_compile_definitions(GLM_FORCE_PRECISION_HIGHP_DOUBLE)
#add_compile_definitions(GLM_FORCE_PRECISION_LOWP_INT)
#add_compile_definitions(GLM_FORCE_PRECISION_MEDIUMP_INT)
#add_compile_definitions(GLM_FORCE_PRECISION_HIGHP_INT)
#add_compile_definitions(GLM_FORCE_PRECISION_LOWP_UINT)
#add_compile_definitions(GLM_FORCE_PRECISION_MEDIUMP_UINT)
#add_compile_definitions(GLM_FORCE_PRECISION_HIGHP_UINT)
#add_compile_definitions(GLM_FORCE_SINGLE_ONLY)
#add_compile_definitions(GLM_FORCE_SWIZZLE)
#add_compile_definitions(GLM_FORCE_XYZW_ONLY)
#add_compile_definitions(GLM_FORCE_LEFT_HANDED)
#add_compile_definitions(GLM_FORCE_DEPTH_ZERO_TO_ONE)
#add_compile_definitions(GLM_FORCE_SIZE_T_LENGTH)
#add_compile_definitions(GLM_FORCE_UNRESTRICTED_GENTYPE)
#add_compile_definitions(GLM_FORCE_SILENT_WARNINGS)
#add_compile_definitions(GLM_FORCE_QUAT_DATA_WXYZ)


# GLEW Options
set(GLEW_VERBOSE OFF)
set(GLEW_USE_STATIC_LIBS NOT ${BUILD_SHARED_LIBS})

# GLFW Options
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_INSTALL OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_VULKAN_STATIC OFF)
if (WINDOWS)
	set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF)
	set(GLFW_USE_HYBRID_HPG OFF)
endif ()
if (LINUX)
	set(GLFW_USE_WAYLAND OFF)
endif ()

# SpdLog Options

# VkBootstrap Options

# VMA Options
set(VMA_RECORDING_ENABLED)
set(VMA_USE_STL_CONTAINERS)
set(VMA_STATIC_VULKAN_FUNCTIONS)
set(VMA_DYNAMIC_VULKAN_FUNCTIONS)
set(VMA_DEBUG_ALWAYS_DEDICATED_MEMORY)
set(VMA_DEBUG_INITIALIZE_ALLOCATIONS)
set(VMA_DEBUG_GLOBAL_MUTEX)
set(VMA_DEBUG_DONT_EXCEED_MAX_MEMORY_ALLOCATION_COUNT)

function(FindLocalLibComponents NAME MODULE MIN_VERSION MAX_VERSION REQUIRED_COMPONENTS)
	# Reject packages that don't work with this versioning system.
	if (${MIN_VERSION} STREQUAL "NULL" AND ${MAX_VERSION} STREQUAL "NULL")
		message(STATUS "Skipping local dependency searching for ${NAME}.")
		return()
	endif ()

	# Use basic built-in CMake package finding.
	string(REPLACE " " ";" REQUIRED_COMPONENTS "${REQUIRED_COMPONENTS}")
	find_package(${NAME} ${MIN_VERSION}...${MAX_VERSION} QUIET COMPONENTS ${REQUIRED_COMPONENTS})

	# If a valid version is not found and PKG_CONFIG is found use PKG_CONFIG too look for a valid version of the package.
	if (NOT ${NAME}_FOUND AND PKG_CONFIG_FOUND)
		pkg_search_module(${NAME} ${MODULE} ${MIN_VERSION}...${MAX_VERSION} QUIET COMPONENTS ${REQUIRED_COMPONENTS})
	endif ()

	# If a valid version is found message accordingly.
	if (${NAME}_FOUND)
		if (${NAME}_VERSION)
			message(STATUS "Using local ${NAME} version ${${NAME}_VERSION}.")
		else ()
			message(STATUS "Using local ${NAME}.")
		endif ()
	else ()
		if (${NAME}_VERSION)
			message(STATUS "Found local ${NAME} of invalid version v${${NAME}_VERSION}. Required version: ${MIN_VERSION}...${MAX_VERSION}")
		endif ()
	endif ()

	# Update variables in the parent scope.
	set(${NAME}_FOUND ${${NAME}_FOUND} PARENT_SCOPE)
	set(${NAME}_VERSION ${${NAME}_VERSION} PARENT_SCOPE)
endfunction()

function(FindLocalLib NAME MODULE MIN_VERSION MAX_VERSION)
	FindLocalLibComponents(${NAME} ${MODULE} ${MIN_VERSION} ${MAX_VERSION} "" "")
endfunction()

function(FetchRemoteLib NAME DOWNLOAD_ARGS)
	# The last item in the download args should be the
	string(REPLACE " " ";" ARGS ${DOWNLOAD_ARGS})
	list(GET ARGS -1 VERSION)

	# This will always be needed to build the package even if it is already downloaded.
	FetchContent_Declare(${NAME} ${ARGS})

	# Check old version information
	file(APPEND ${IE_DEPS_DOWNLOAD_DIR}/${NAME}Version)
	file(READ ${IE_DEPS_DOWNLOAD_DIR}/${NAME}Version OLD_VERSION)

	# Download content if needed
	if (NOT OLD_VERSION STREQUAL VERSION)
		FetchContent_Populate(${NAME} SUBBUILD_DIR ${IE_DEPS_BUILD_DIR}/${NAME}_subbuild SOURCE_DIR ${IE_DEPS_DOWNLOAD_DIR}/${NAME}_src BINARY_DIR ${IE_DEPS_BUILD_DIR}/${NAME}_build ${ARGS})

		# Update version information
		file(WRITE ${IE_DEPS_DOWNLOAD_DIR}/${NAME}Version ${VERSION})
	endif ()

	# Message package source.
	message(STATUS "Using remote ${NAME} version ${VERSION}.")
endfunction()

# Lock the Download Guard
file(LOCK ${IE_DOWNLOAD_GUARD_FILE} RESULT_VARIABLE HAS_DOWNLOAD_GUARD_LOCK TIMEOUT 0)
if (NOT ${HAS_DOWNLOAD_GUARD_LOCK} MATCHES 0)
	# Wait until the Download Guard is unlocked
	file(LOCK ${IE_DOWNLOAD_GUARD_FILE})
endif ()

# FetchContent loop is guarded by the Download Guard
# Do all the downloads here. They will only run in a single CMake instance at a time no matter how many are running.
foreach (LIST ${GITHUB})
	list(GET ${LIST} ${GITHUB_NAME} NAME)
	if (${IE_PREFER_LOCAL_LIBS})
		list(GET ${LIST} ${GITHUB_MODULE} MODULE)
		list(GET ${LIST} ${GITHUB_MIN_VERSION} MIN_VERSION)
		list(GET ${LIST} ${GITHUB_MAX_VERSION} MAX_VERSION)
		FindLocalLib(${NAME} ${MODULE} ${MIN_VERSION} ${MAX_VERSION})
	endif ()
	if (NOT ${NAME}_FOUND)
		list(GET ${LIST} ${GITHUB_DOWNLOAD_ARGS} ARGS)
		FetchRemoteLib(${NAME} ${ARGS})
	endif ()
endforeach ()

foreach (LIST ${URL})
	list(GET ${LIST} ${URL_NAME} NAME)
	if (${IE_PREFER_LOCAL_LIBS})
		list(GET ${LIST} ${URL_MODULE} MODULE)
		list(GET ${LIST} ${URL_MIN_VERSION} MIN_VERSION)
		list(GET ${LIST} ${URL_MAX_VERSION} MAX_VERSION)
		FindLocalLib(${NAME} ${MODULE} ${MIN_VERSION} ${MAX_VERSION})
	endif ()
	if (NOT ${NAME}_FOUND)
		list(GET ${LIST} ${URL_DOWNLOAD_ARGS} ARGS)
		FetchRemoteLib(${NAME} ${ARGS})
	endif ()
endforeach ()

# Unlock the Download Guard
file(REMOVE ${IE_DOWNLOAD_GUARD_FILE})

# add_subdirectory( calls should go after the Download Guard
foreach (LIST ${GITHUB})
	list(GET ${LIST} ${GITHUB_MODULE} PKG_NAME)
	list(GET ${LIST} ${GITHUB_NAME} NAME)
	list(GET ${LIST} ${GITHUB_EXPORT_LIB} EXPORT_LIB)

	# If the export lib has not already been made, create it with the empty main.cpp file.
	if (NOT TARGET ${EXPORT_LIB})
		add_library(${EXPORT_LIB} main.cpp)
	endif ()

	# Is a remote dependency
	if (NOT ${NAME}_FOUND)
		list(GET ${LIST} ${GITHUB_BUILD_DIR} CMAKE_DIR)
		list(GET ${LIST} ${GITHUB_INCLUDE_DIR} INCLUDE_DIR)
		add_subdirectory(${IE_DEPS_DOWNLOAD_DIR}/${NAME}_src${CMAKE_DIR})
		target_include_directories(${EXPORT_LIB} PUBLIC ${IE_DEPS_DOWNLOAD_DIR}/${NAME}_src${INCLUDE_DIR})
	endif ()

	# Export requested targets to requested export lib.
	if (TARGET ${PKG_NAME})
		target_link_libraries(${EXPORT_LIB} PUBLIC ${PKG_NAME})
	else ()
		if (TARGET ${NAME})
			target_link_libraries(${EXPORT_LIB} PUBLIC ${NAME})
		else ()
			target_link_libraries(${EXPORT_LIB} PUBLIC ${${NAME}_LIBRARIES})
			target_include_directories(${EXPORT_LIB} PUBLIC ${${NAME}_INCLUDE_DIRS})
		endif ()
	endif ()
endforeach ()

foreach (LIST ${URL})
	list(GET ${LIST} ${URL_MODULE} PKG_NAME)
	list(GET ${LIST} ${URL_NAME} NAME)
	list(GET ${LIST} ${URL_EXPORT_LIB} EXPORT_LIB)

	# If the export lib has not already been made, create it with the empty main.cpp file.
	if (NOT TARGET ${EXPORT_LIB})
		add_library(${EXPORT_LIB} main.cpp)
	endif ()

	# Is a remote dependency
	if (NOT ${NAME}_FOUND)
		list(GET ${LIST} ${URL_BUILD_DIR} CMAKE_DIR)
		list(GET ${LIST} ${URL_INCLUDE_DIR} INCLUDE_DIR)
		add_subdirectory(${IE_DEPS_DOWNLOAD_DIR}/${NAME}_src${CMAKE_DIR})
		target_include_directories(${EXPORT_LIB} PUBLIC ${IE_DEPS_DOWNLOAD_DIR}/${NAME}_src${INCLUDE_DIR})
	endif ()

	# Export requested targets to requested export lib.
	if (TARGET ${PKG_NAME})
		target_link_libraries(${EXPORT_LIB} PUBLIC ${PKG_NAME})
	else ()
		if (TARGET ${NAME})
			target_link_libraries(${EXPORT_LIB} PUBLIC ${NAME})
		else ()
			target_link_libraries(${EXPORT_LIB} PUBLIC ${${NAME}_LIBRARIES})
			target_include_directories(${EXPORT_LIB} PUBLIC ${${NAME}_INCLUDE_DIRS})
		endif ()
	endif ()
endforeach ()

foreach (LIST ${LOCAL})
	# If the package has no download arguments, then it must be in local storage
	list(GET ${LIST} ${LOCAL_NAME} NAME)
	list(GET ${LIST} ${LOCAL_MODULE} MODULE)
	list(GET ${LIST} ${LOCAL_REQUIRED_COMPONENTS} REQUIRED_COMPONENTS)
	list(GET ${LIST} ${LOCAL_MIN_VERSION} MIN_VERSION)
	list(GET ${LIST} ${LOCAL_MAX_VERSION} MAX_VERSION)
	FindLocalLibComponents(${NAME} ${MODULE} ${MIN_VERSION} ${MAX_VERSION} "${REQUIRED_COMPONENTS}")
	if (NOT ${NAME}_FOUND)
		message(STATUS "Failed to find any version of ${NAME}! Building the project will likely fail during linking.")
	endif ()

	# Add to build
	list(GET ${LIST} ${LOCAL_EXPORT_LIB} EXPORT_LIB)

	# Prepare components
	string(STRIP "${REQUIRED_COMPONENTS}" COMPONENTS)
	string(REPLACE " " ";" COMPONENTS "${COMPONENTS}")
	set(COMPONENT_LIBRARIES "")

	# If the export lib has not already been made, create it with the empty main.cpp file.
	if (NOT TARGET ${EXPORT_LIB})
		add_library(${EXPORT_LIB} main.cpp)
	endif ()

	# Build list of components
	foreach (COMPONENT ${COMPONENTS})
		string(PREPEND COMPONENT "${NAME}::")
		list(APPEND COMPONENT_LIBRARIES "${COMPONENT}")
	endforeach ()

	# Export requested targets to requested export lib.
	if (TARGET ${MODULE})
		target_link_libraries(${EXPORT_LIB} PUBLIC ${MODULE} ${COMPONENT_LIBRARIES})
	else ()
		if (TARGET ${NAME}::${NAME})
			target_link_libraries(${EXPORT_LIB} PUBLIC ${NAME} ${COMPONENT_LIBRARIES})
		else ()
			target_link_libraries(${EXPORT_LIB} PUBLIC ${${NAME}_LIBRARIES} ${COMPONENT_LIBRARIES})
			target_include_directories(${EXPORT_LIB} PUBLIC ${${NAME}_INCLUDE_DIRS})
		endif ()
	endif ()
endforeach ()

add_library(INT_src main.cpp)
target_include_directories(INT_src PUBLIC ${IE_BASE_DIR}/src)

if (CLANG)
	target_compile_options(vma PUBLIC "-Wnullability-completeness")
endif ()
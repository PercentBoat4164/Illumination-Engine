find_package(Vulkan REQUIRED COMPONENTS glslc)
find_package(OpenGL REQUIRED)

include(FetchContent)  # Requires CMake 3.14

set(IE_DEPS_DOWNLOAD_DIR   ${IE_BASE_DIR}/ext/deps)
set(IE_DEPS_BUILD_DIR      ${CMAKE_BINARY_DIR}/deps)
set(IE_DOWNLOAD_GUARD_FILE ${CMAKE_SOURCE_DIR}/__DOWNLOAD_GUARD__)

set(FETCHCONTENT_BASE_DIR ${IE_DEPS_DOWNLOAD_DIR})

# Possibly add https://github.com/libvips/libvips as the image importer of choice. (high speed, high compatibility, highly maintained, but does not use CMake)

# package_DATA =     NAME        MODULE_NAME  BUILD_DIR      INCLUDE_DIR DOWNLOAD_ARGS
set(assimp_DATA      assimp      assimp       ""             "/include"  GIT_REPOSITORY https://github.com/assimp/assimp                                               GIT_PROGRESS ON GIT_TAG v5.2.5)
set(glm_DATA         glm         glm          ""             ""          GIT_REPOSITORY https://github.com/g-truc/glm                                                  GIT_PROGRESS ON GIT_TAG master)
set(glew_DATA        glew        glew         "/build/cmake" "/include"  URL            https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.tgz)
set(glfw_DATA        GLFW        glfw3        ""             "/include"  GIT_REPOSITORY https://github.com/glfw/glfw                                                   GIT_PROGRESS ON GIT_TAG 3.3.8)
set(spdlog_DATA      spdlog      spdlog       ""             "/include"  GIT_REPOSITORY https://github.com/gabime/spdlog                                               GIT_PROGRESS ON GIT_TAG v1.x)
set(VkBootstrap_DATA VkBootstrap vk-bootstrap ""             "/src"      GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap                                 GIT_PROGRESS ON GIT_TAG v0.5)
set(vma_DATA         vma         vma          ""             "/include"  GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator              GIT_PROGRESS ON GIT_TAG v3.0.1)
set(DATA assimp_DATA glm_DATA glew_DATA glfw_DATA spdlog_DATA VkBootstrap_DATA vma_DATA)


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

include(FindPkgConfig)

# Lock the Download Guard
file(LOCK ${IE_DOWNLOAD_GUARD_FILE} RESULT_VARIABLE HAS_DOWNLOAD_GUARD_LOCK TIMEOUT 0)
if (NOT ${HAS_DOWNLOAD_GUARD_LOCK} MATCHES 0)
	# Wait until the Download Guard is unlocked
	file(LOCK ${IE_DOWNLOAD_GUARD_FILE})
endif ()

# FetchContent loop is guarded by the Download Guard
foreach (LIST ${DATA})
	list(GET ${LIST} 0 NAME)
	list(SUBLIST ${LIST} 3 -1 ARGS)
	FetchContent_Declare(${NAME} ${ARGS})
endforeach ()


# Do all the downloads here. They will only run in a single CMake instance at a time no matter how many are running.
foreach (LIST ${DATA})
	list(GET ${LIST} 0 NAME)

	if (${IE_PREFER_LOCAL_LIBS})
		find_package(${NAME} QUIET)
		if (${NAME}_FOUND)
			message("Using local ${NAME} version ${${NAME}_VERSION}.")
		else ()
			list(GET ${LIST} 1 PACKAGE)
			pkg_search_module(${NAME} ${PACKAGE} QUIET)
			if (${NAME}_FOUND)
				message("Using local ${NAME} version ${${NAME}_VERSION}.")
			endif ()
		endif ()
	endif ()

	if (NOT ${NAME}_FOUND)
		list(GET ${LIST} -1 VERSION)
		list(SUBLIST ${LIST} 4 -1 ARGS)

		# Check old version information
		file(APPEND ${IE_DEPS_DOWNLOAD_DIR}/${NAME}Version)
		file(READ ${IE_DEPS_DOWNLOAD_DIR}/${NAME}Version OLD_VERSION)

		# Download content if needed
		if (NOT OLD_VERSION STREQUAL VERSION)
			FetchContent_Populate(${NAME} SUBBUILD_DIR ${IE_DEPS_BUILD_DIR}/${NAME}_subbuild SOURCE_DIR ${IE_DEPS_DOWNLOAD_DIR}/${NAME}_src BINARY_DIR ${IE_DEPS_BUILD_DIR}/${NAME}_build ${ARGS})

			# Update version information
			file(WRITE ${IE_DEPS_DOWNLOAD_DIR}/${NAME}Version ${VERSION})
		endif ()
	endif ()
endforeach ()

# Unlock the Download Guard
file(REMOVE ${IE_DOWNLOAD_GUARD_FILE})

# add_subdirectory( calls should go after the Download Guard
foreach (LIST ${DATA})
	list(GET ${LIST} 0 NAME)
	list(GET ${LIST} 2 CMAKE_DIR)
	list(GET ${LIST} 3 INCLUDE_DIR)
	if (NOT ${NAME}_FOUND)
		add_subdirectory(${IE_DEPS_DOWNLOAD_DIR}/${NAME}_src/${CMAKE_DIR})
		list(APPEND INCLUDE_DIRS ${IE_DEPS_DOWNLOAD_DIR}/${NAME}_src${INCLUDE_DIR})
		list(GET ${LIST} 1 PKG_NAME)
		if (TARGET ${PKG_NAME})
			list(APPEND LIBRARIES ${PKG_NAME})
		endif ()
	else ()
		list(APPEND INCLUDE_DIRS ${${NAME}_INCLUDE_DIRS})
		list(APPEND LIBRARIES ${${NAME}_LIBRARIES})
	endif ()
endforeach ()

set(LIBRARIES ${LIBRARIES} ${Vulkan_LIBRARIES} PARENT_SCOPE)
set(INCLUDE_DIRS ${INCLUDE_DIRS} ${IE_BASE_DIR} PARENT_SCOPE)

set(Vulkan_LIBRARIES ${Vulkan_LIBRARIES} PARENT_SCOPE)
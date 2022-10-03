# @todo Select individual versions for each library.
# @todo Populate all options available in each library.
# @todo Allow usage of local libraries instead of remotes.
find_package(Vulkan REQUIRED COMPONENTS glslc)
find_package(OpenGL REQUIRED)

include(FetchContent)  # Requires CMake 3.14

set(IE_DEPS_DOWNLOAD_DIR   ${IE_BASE_DIR}/ext/deps)
set(IE_DEPS_BUILD_DIR      ${CMAKE_BINARY_DIR}/deps)
set(IE_DOWNLOAD_GUARD_FILE ${CMAKE_SOURCE_DIR}/__DOWNLOAD_GUARD__)

set(FETCHCONTENT_BASE_DIR ${IE_DEPS_DOWNLOAD_DIR})

# Possibly add https://github.com/libvips/libvips as the image importer of choice. (high speed, high compatibility, highly maintained, but does not use CMake)

# package_DATA = $NAME $CMAKE_BUILD_LOCATION $INCLUDE_DIR $DOWNLOAD_ARGS
set(assimp_DATA      assimp      ""             ""     GIT_REPOSITORY https://github.com/assimp/assimp                                               GIT_PROGRESS ON GIT_TAG master)
set(glm_DATA         glm         ""             ""     GIT_REPOSITORY https://github.com/g-truc/glm                                                  GIT_PROGRESS ON GIT_TAG master)
set(glew_DATA        glew        "/build/cmake" ""     URL            https://github.com/nigels-com/glew/releases/download/glew-2.2.0/glew-2.2.0.tgz)
set(glfw_DATA        glfw        ""             ""     GIT_REPOSITORY https://github.com/glfw/glfw                                                   GIT_PROGRESS ON GIT_TAG master)
set(spdlog_DATA      spdlog      ""             ""     GIT_REPOSITORY https://github.com/gabime/spdlog                                               GIT_PROGRESS ON GIT_TAG v1.x)
set(VkBootstrap_DATA VkBootstrap ""             "/src" GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap                                 GIT_PROGRESS ON GIT_TAG master)
set(vma_DATA         vma         ""             ""     GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator              GIT_PROGRESS ON GIT_TAG master)
set(DATA assimp_DATA glm_DATA glew_DATA glfw_DATA spdlog_DATA VkBootstrap_DATA vma_DATA)


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
else ()
	set(ASSIMP_BUILD_ASSIMP_VIEW OFF)
endif ()

# GLEW Options
set(GLEW_VERBOSE OFF)
set(GLEW_USE_STATIC_LIBS NOT ${BUILD_SHARED_LIBS})

# GLFW Options
set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_INSTALL OFF)
set(GLFW_BUILD_EXAMPLES OFF)


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
	list(GET ${LIST} -1 VERSION)
	list(SUBLIST ${LIST} 3 -1 ARGS)

	# Check old version information
	file(APPEND ${IE_DEPS_DOWNLOAD_DIR}/${NAME}Version)
	file(READ ${IE_DEPS_DOWNLOAD_DIR}/${NAME}Version OLD_VERSION)

	# Download content if needed
	if (NOT OLD_VERSION STREQUAL VERSION)
		FetchContent_Populate(${NAME} SUBBUILD_DIR ${IE_DEPS_BUILD_DIR}/${NAME}_subbuild SOURCE_DIR ${IE_DEPS_DOWNLOAD_DIR}/${NAME}_src BINARY_DIR ${IE_DEPS_BUILD_DIR}/${NAME}_build ${ARGS})
	endif ()

	# Update version information
	file(WRITE ${IE_DEPS_DOWNLOAD_DIR}/${NAME}Version ${VERSION})
endforeach ()

# Unlock the Download Guard
file(REMOVE ${IE_DOWNLOAD_GUARD_FILE})

# add_subdirectory( calls should go after the Download Guard
foreach (LIST ${DATA})
	list(GET ${LIST} 0 NAME)
	list(GET ${LIST} 1 CMAKE_DIR)
	list(GET ${LIST} 2 INCLUDE_DIR)
	add_subdirectory(${IE_DEPS_DOWNLOAD_DIR}/${NAME}_src/${CMAKE_DIR})
	list(APPEND INCLUDE_DIRS ${IE_DEPS_DOWNLOAD_DIR}/${NAME}_src${INCLUDE_DIR})
endforeach ()

set(INCLUDE_DIRS ${INCLUDE_DIRS} ${IE_BASE_DIR} PARENT_SCOPE)

set(Vulkan_LIBRARIES ${Vulkan_LIBRARIES} PARENT_SCOPE)
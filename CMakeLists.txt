cmake_minimum_required(VERSION 3.10.2)
project(CrystalEngine)

# Set CMake variables
set(OpenGL_GL_PREFERENCE LEGACY)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

# Get packages
find_package(Vulkan)
find_package(OpenGL REQUIRED)
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
endif(CCACHE_FOUND)

# Specify no docs, tests, or examples from GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Extract dependencies
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf otherSources.tar.xz WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deps)
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf vk-bootstrap.tar.xz WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deps)
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf glfw.tar.xz WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deps)
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf glm.tar.xz WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deps)
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf glew.tar.xz WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deps)

# Add dependencies
add_subdirectory(deps/glm)
add_subdirectory(deps/glfw)
add_subdirectory(deps/glew/build/cmake)
add_subdirectory(deps/vk-bootstrap)

# Download Models if they don't already exist
if(NOT EXISTS "${CMAKE_SOURCE_DIR}/src/Models")
    execute_process(COMMAND python3 "${CMAKE_SOURCE_DIR}/DownloadModels.py" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
endif()

# Copy data files and dlls
set(data_SHADOW src/Shaders src/Models deps/glew32d.dll deps/glew32.dll deps/vulkan-1.dll deps/libvulkan.so.1)
foreach(item IN LISTS data_SHADOW)
    message("Copying '" ${item} "'")
    FILE(COPY ${item} DESTINATION "${CMAKE_BINARY_DIR}")
endforeach()

# Generate executable
add_executable(CrystalEngine src/main.cpp src/GraphicsEngine/Vulkan/AccelerationStructureManager.hpp src/GraphicsEngine/Vulkan/ShaderBindingTableManager.hpp src/GraphicsEngine/Vulkan/DescriptorSetManager.hpp)
target_link_libraries(CrystalEngine PUBLIC glew ${GLEW_LIBRARIES} glfw ${GLFW_LIBRARIES} glm ${GLM_LIBRARIES} vk-bootstrap ${VK_BOOTSTRAP_LIBRARIES} Vulkan::Vulkan)

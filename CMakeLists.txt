cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)
project(IlluminationEngine)

# Set CMake variables
set(OpenGL_GL_PREFERENCE LEGACY)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(ILLUMINATION_ENGINE_PREFER_PRE_INSTALLED false)

# Get packages
find_package(OpenGL REQUIRED) # install with "sudo apt-get install libglu1-mesa-dev mesa-common-dev"
if (ILLUMINATION_ENGINE_PREFER_PRE_INSTALLED)
    find_package(Vulkan) # install with "wget -qO - https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo apt-key add -;sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-1.2.176-focal.list https://packages.lunarg.com/vulkan/1.2.176/lunarg-vulkan-1.2.176-focal.list;sudo apt update;sudo apt install vulkan-sdk"
    find_package(GLEW) # install with "sudo apt install libglew-dev"
    find_package(glfw3 3.3) # install with "sudo apt install libglfw3-dev"
    find_package(glm) # install with "sudo apt install libglm-dev"
endif()

# Specify no docs, tests, or examples from GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

# Extract dependencies
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf sources.tar.xz WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deps)

# Copy data files and dlls
FILE(COPY res DESTINATION "${CMAKE_BINARY_DIR}")
if (NOT $ENV{CLION_IDE})
    FILE(COPY build/bin/Debug/glew32d.dll DESTINATION "${CMAKE_BINARY_DIR}/Debug")
endif()

# Generate executable
add_executable(IlluminationEngine src/main.cpp src/GraphicsEngine/OpenGL/openglVertex.hpp)

# Add dependencies
if (NOT Vulkan_FOUND)
#    if (WIN32)
#        find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan HINTS "${CMAKE_SOURCE_DIR}/deps/vulkanHeaders")
#    else()
#        find_library(Vulkan_LIBRARY NAMES vulkan HINTS "${CMAKE_SOURCE_DIR}/deps/vulkanHeaders")
#    endif()
#    message("Building with packaged Vulkan")
#    link_libraries(${Vulkan_LIBRARY})
else()
    message("Building with pre-installed Vulkan")
endif()
if (NOT glm_FOUND)
    add_subdirectory(deps/glm)
    message("Building with packaged glm")
    target_link_libraries(IlluminationEngine PUBLIC glm)
else()
    message("Building with pre-installed glm")
    target_link_libraries(IlluminationEngine PUBLIC glm)
endif()
if (NOT glfw3_FOUND)
    add_subdirectory(deps/glfw)
    include_directories(deps/glfw/include)
    target_link_libraries(IlluminationEngine PUBLIC glfw)
    message("Building with packaged glfw")
else()
    target_link_libraries(IlluminationEngine PUBLIC glfw)
    message("Building with pre-installed glfw")
endif()
if (NOT GLEW_FOUND)
    add_subdirectory(deps/glew/build/cmake)
    include_directories(deps/glew/include)
    target_link_libraries(IlluminationEngine PUBLIC glew)
    message("Building with packaged glew")
else()
    target_link_libraries(IlluminationEngine PUBLIC GLEW::GLEW)
    message("Building with pre-installed glew")
endif()
if (Vulkan_FOUND)
    add_subdirectory(deps/vk-bootstrap)
    target_link_libraries(IlluminationEngine PUBLIC vk-bootstrap Vulkan::Vulkan)
endif()
target_link_libraries(IlluminationEngine PUBLIC OpenGL::GL)

# Add compile definitions based on supported features
add_compile_definitions(ILLUMINATION_ENGINE_OPENGL)
if (Vulkan_FOUND)
    add_compile_definitions(ILLUMINATION_ENGINE_VULKAN)
    add_compile_definitions(ILLUMINATION_ENGINE_VULKAN_RAY_TRACING)
endif()

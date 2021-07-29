cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)
project(IlluminationEngine)

# Set CMake variables
set(OpenGL_GL_PREFERENCE LEGACY)
set(CMAKE_CXX_STANDARD 23)
#set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS False)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(ILLUMINATION_ENGINE_PREFER_PRE_INSTALLED true)

# Get packages
find_package(OpenGL REQUIRED) # install with "sudo apt-get install libglu1-mesa-dev mesa-common-dev"
find_package(Vulkan) # install with "wget -qO - https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo apt-key add -;sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-1.2.176-focal.list https://packages.lunarg.com/vulkan/1.2.176/lunarg-vulkan-1.2.176-focal.list;sudo apt update;sudo apt install vulkan-sdk"
if (ILLUMINATION_ENGINE_PREFER_PRE_INSTALLED)
    find_package(GLEW) # install with "sudo apt install libglew-dev"
#    find_package(glfw3) # install with "sudo apt install libglfw3-dev" COMMENTED BECAUSE IT FAILS TO LINK AGAINST PRE-INSTALLED VERSIONS
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
add_executable(IlluminationEngine src/main.cpp)

# Add dependencies
if (NOT glm_FOUND)
    add_subdirectory(deps/glm)
    message("Building with packaged glm")
else()
    message("Building with pre-installed glm")
endif()
target_link_libraries(IlluminationEngine PUBLIC glm::glm)
if (NOT glfw3_FOUND)
    add_subdirectory(deps/glfw)
    include_directories(deps/glfw/include)
    message("Building with packaged glfw")
else()
    message("Building with pre-installed glfw")
endif()
target_link_libraries(IlluminationEngine PUBLIC glfw)
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
endif()
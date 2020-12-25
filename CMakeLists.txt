cmake_minimum_required(VERSION 3.10.2)
project(GameEngine)

set(CMAKE_CXX_STANDARD 17)
find_package(Vulkan REQUIRED)
find_package(ArrayFire REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf glfw.tar.xz WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Sources)

add_subdirectory(Sources/glfw)
add_executable(GameEngine Renderer.cpp)
target_link_libraries(GameEngine glfw ${GLFW_LIBRARIES} Vulkan::Vulkan ArrayFire::af)
cmake_minimum_required(VERSION 3.10.2)
project(GameEngine)

set(CMAKE_CXX_STANDARD 17)
find_package(Vulkan REQUIRED)
find_package(ArrayFire REQUIRED)
add_subdirectory(Sources/glfw)

add_executable(GameEngine Renderer.cpp)
target_link_libraries(GameEngine glfw ${GLFW_LIBRARIES} Vulkan::Vulkan ArrayFire::af)
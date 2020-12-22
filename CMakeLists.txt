cmake_minimum_required(VERSION 3.10.2)
project(vulkantutorial)

set(CMAKE_CXX_STANDARD 17)
find_package(Vulkan REQUIRED)
add_subdirectory(Sources/glfw)
find_package(ArrayFire REQUIRED)

add_executable(vulkantutorial Renderer.cpp)
target_link_libraries(vulkantutorial glfw ${GLFW_LIBRARIES} Vulkan::Vulkan ArrayFire::af)
cmake_minimum_required(VERSION 3.10.2)
project(GameEngine)

set(OpenGL_GL_PREFERENCE GLVND)
set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan)
find_package(OpenGL REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf glfw.tar.xz WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/sources)
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf glew.tar.xz WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/sources)
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf glm.tar.xz WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/sources)

add_subdirectory(sources/glm)
add_subdirectory(sources/glfw)
add_subdirectory(sources/glew/build/cmake)
add_executable(Main main.cpp)
add_executable(Physics Physics.cpp)

target_link_libraries(Main glew ${GLEW_LIBRARIES} glfw ${GLFW_LIBRARIES} Vulkan::Vulkan glm ${GLM_LIBRARIES})
target_link_libraries(Physics glm ${GLM_LIBRARIES})
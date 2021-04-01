cmake_minimum_required(VERSION 3.10.2)
project(CrystalEngine)

set(OpenGL_GL_PREFERENCE GLVND)
set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan)
find_package(OpenGL REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf otherSources.tar.xz WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/sources)
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf vk-bootstrap.tar.xz WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/sources)
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf glfw.tar.xz WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/sources)
execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf glm.tar.xz WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/sources)

execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf glew.tar.xz WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/sources)

add_subdirectory(sources/glm)
add_subdirectory(sources/glfw)
add_subdirectory(sources/glew/build/cmake)
add_subdirectory(sources/vk-bootstrap)

set(data_SHADOW shaders models sources/glew32d.dll sources/vulkan-1.dll)
foreach(item IN LISTS data_SHADOW)
    message("Copying '" ${item} "'")
    FILE(COPY ${item} DESTINATION "${CMAKE_BINARY_DIR}")
endforeach()

add_executable(CrystalEngine main.cpp)
target_link_libraries(CrystalEngine glew ${GLEW_LIBRARIES} glfw ${GLFW_LIBRARIES} glm ${GLM_LIBRARIES} vk-bootstrap ${VK_BOOTSTRAP_LIBRARIES} Vulkan::Vulkan)
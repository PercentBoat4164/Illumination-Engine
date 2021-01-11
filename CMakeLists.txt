cmake_minimum_required(VERSION 3.10.2)
project(GameEngine)

set(CMAKE_CXX_STANDARD 17)
find_package(Vulkan REQUIRED)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf glfw.tar.xz WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/sources)

IF(WIN32)
    add_subdirectory(sources/glm)
ENDIF()

add_subdirectory(sources/glfw)
add_executable(Main main.cpp)
add_executable(RenderEngineOld RenderEngineOld.cpp)
add_executable(RenderEngine RenderEngine.cpp)
add_executable(Physics Physics.cpp)

IF(WIN32)
    target_link_libraries(Main glfw ${GLFW_LIBRARIES} Vulkan::Vulkan glm ${GLM_LIBRARIES})
    target_link_libraries(RenderEngineOld glfw ${GLFW_LIBRARIES} Vulkan::Vulkan glm ${GLM_LIBRARIES})
    target_link_libraries(RenderEngine glfw ${GLFW_LIBRARIES} Vulkan::Vulkan glm ${GLM_LIBRARIES})
ELSE()
    target_link_libraries(Main glfw ${GLFW_LIBRARIES} Vulkan::Vulkan)
    target_link_libraries(RenderEngineOld glfw ${GLFW_LIBRARIES} Vulkan::Vulkan)
    target_link_libraries(RenderEngine glfw ${GLFW_LIBRARIES} Vulkan::Vulkan)
ENDIF()
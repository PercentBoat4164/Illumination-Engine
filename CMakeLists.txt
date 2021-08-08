cmake_minimum_required(VERSION 3.10.2 FATAL_ERROR)
project(IlluminationEngine)

set(OpenGL_GL_PREFERENCE LEGACY)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_EXTENSIONS false)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(ILLUMINATION_ENGINE_PREFER_PRE_INSTALLED false)

# Note: All install instructions are for Ubuntu Linux
find_package(OpenGL REQUIRED) # install with "sudo apt install libglu1-mesa-dev mesa-common-dev"
find_package(Vulkan) # install with "wget -qO - https://packages.lunarg.com/lunarg-signing-key-pub.asc | sudo apt-key add -;sudo wget -qO /etc/apt/sources.list.d/lunarg-vulkan-1.2.182-focal.list https://packages.lunarg.com/vulkan/1.2.176/lunarg-vulkan-1.2.182-focal.list;sudo apt update;sudo apt install vulkan-sdk"
if (ILLUMINATION_ENGINE_PREFER_PRE_INSTALLED)
    find_package(GLEW) # install with "sudo apt install libglew-dev"
    find_package(glfw3) # install with "sudo apt install libglfw3-dev"
    find_package(glm) # install with "sudo apt install libglm-dev"
    find_package(assimp) # install with "sudo apt install assimp"
endif()

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)

execute_process(WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} COMMAND ${CMAKE_COMMAND} -E tar xzf deps.tar.xz)

add_executable(IlluminationEngine src/main.cpp)

if (NOT assimp_FOUND)
    add_subdirectory(deps/assimp)
    message("Building with packaged assimp")
else()
    message("Building with pre-installed assimp")
endif()
target_link_libraries(IlluminationEngine PUBLIC assimp::assimp)
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

add_compile_definitions(ILLUMINATION_ENGINE_OPENGL)
if (Vulkan_FOUND)
    add_compile_definitions(ILLUMINATION_ENGINE_VULKAN)
endif()

execute_process(COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/res)
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res ${CMAKE_BINARY_DIR}/res)
if (WIN32)
    if ($ENV{CLION_IDE})
        if (CMAKE_BUILD_TYPE STREQUAL Debug)
            add_custom_command(TARGET IlluminationEngine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/bin/glew32d.dll ${CMAKE_BINARY_DIR})
            add_custom_command(TARGET IlluminationEngine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/deps/glfw/src/glfw3d.dll ${CMAKE_BINARY_DIR})
            add_custom_command(TARGET IlluminationEngine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/deps/assimp/code/assimp-vc142-mtd.dll ${CMAKE_BINARY_DIR})
        else ()
            add_custom_command(TARGET IlluminationEngine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/bin/glew32.dll ${CMAKE_BINARY_DIR})
            add_custom_command(TARGET IlluminationEngine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/deps/glfw/src/glfw3.dll ${CMAKE_BINARY_DIR})
            add_custom_command(TARGET IlluminationEngine POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/deps/assimp/code/assimp-vc142-mt.dll ${CMAKE_BINARY_DIR})
        endif ()
    else ()
        # Add Visual Studio file copies here. Change the else above to check specifically for Visual Studio. When doing the if statement for debug compiling, be careful because it is case sensitive. The case used in CLion may not be the same as is used in Visual Studio.
    endif ()
endif ()
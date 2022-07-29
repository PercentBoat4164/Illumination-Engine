set(IEGMVulkanShaderSourceFiles  # Gather sources
        fragmentShader.frag
        vertexShader.vert
        )

add_custom_target(IEGMVulkanShaders)  # Create the custom target

find_program(GLSLC glslc glslc.exe)  # Find glslc (program used to compile shaders)

if (GLSLC)
    set(shader_output_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/Vulkan) # Define destination directory
    file(MAKE_DIRECTORY ${shader_output_directory})  # Create destination directory

    foreach (shader ${IEGMVulkanShaderSourceFiles})
        # Generate command for compiling the shader based on build type
        if (CMAKE_BUILD_TYPE STREQUAL Release)
            add_custom_command(OUTPUT ${shader_output_directory}/${shader}.spv COMMAND ${GLSLC} --target-env=vulkan1.0 ${CMAKE_CURRENT_SOURCE_DIR}/${shader} -o ${shader_output_directory}/${shader}.spv DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${shader} COMMENT "Compiling ${CMAKE_CURRENT_SOURCE_DIR}/${shader} in Release mode.")  # Compile shader in release mode
        else ()
            add_custom_command(OUTPUT ${shader_output_directory}/${shader}.spv COMMAND ${GLSLC} --target-env=vulkan1.0 ${CMAKE_CURRENT_SOURCE_DIR}/${shader} -o ${shader_output_directory}/${shader}.spv DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${shader} COMMENT "Compiling ${CMAKE_CURRENT_SOURCE_DIR}/${shader} in Debug mode.")  # Compile shader in debug mode
        endif ()
        add_custom_command(TARGET IEGMVulkanShaders COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_SOURCE_DIR}/${shader} ${shader_output_directory}/${shader} DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${shader})  # Copy source file to output directory

        target_sources(IEGMVulkanShaders PRIVATE ${shader_output_directory}/${shader}.spv) # Add the compiled output to the sources so that it is regenerated upon change of source
    endforeach ()
endif ()
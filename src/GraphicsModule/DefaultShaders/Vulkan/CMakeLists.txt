set(IEGMVulkanShaderSourceFiles  # Gather sources
		fragmentShader.frag
		vertexShader.vert
		)

add_custom_target(IEGMVulkanShaders)  # Create the custom target

set(shader_output_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/Vulkan) # Define destination directory
file(MAKE_DIRECTORY ${shader_output_directory})  # Create destination directory
foreach (shader ${IEGMVulkanShaderSourceFiles})
	add_custom_command(TARGET IEGMVulkanShaders COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${shader} ${shader_output_directory}/${shader})  # Copy source file to output directory
endforeach ()
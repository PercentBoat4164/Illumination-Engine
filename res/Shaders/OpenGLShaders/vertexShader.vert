#version 140
#extension GL_ARB_explicit_attrib_location : enable

/*
 * This shader supports all GLSL version from 330 to 460 with no extensions.
 * If the GL_ARB_explicit_attrib_location extension is enabled, versions 140 and 150 are also supported.
 * If the GL_ARB_separate_shader_objects extension is enabled, versions 140 and 150 are also supported.
 *
 * Example:
 * #version 140 // Not supported
 *
 * #version 140 // Supported - Lowest supported version
 * #extension GL_ARB_separate_shader_objects : enable
 *
 * #version 460 // Supported - Highest supported version
 */

layout (location=0) in vec3 vertexPosition;
layout (location=1) in vec2 vertexTextureCoordinates;
layout (location=2) in vec3 vertexNormal;

uniform mat4 projection;
uniform mat3 normalMatrix;
uniform mat4 viewModel;
uniform mat4 model;

out vec2 fragmentTextureCoordinates;
out vec3 interpolatedNormal;
out vec3 fragmentPosition;

void main() {
    gl_Position = projection * viewModel * vec4(vertexPosition, 1.0f);
    fragmentTextureCoordinates = vertexTextureCoordinates;
    fragmentPosition = vec3(model * vec4(vertexPosition, 1.0f));
    interpolatedNormal = normalize(normalMatrix * vertexNormal);
}
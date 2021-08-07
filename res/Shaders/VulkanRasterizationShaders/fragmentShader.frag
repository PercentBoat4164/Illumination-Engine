#version 460
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D diffuse;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragmentTextureCoordinates;

layout(location = 0) out vec4 outColor;

//Add normal mapping
//Add POM

void main() {
    outColor = texture(diffuse, fragmentTextureCoordinates);
}
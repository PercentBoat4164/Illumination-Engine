#version 460
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D diffuse;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragmentTextureCoordinates;

layout(location = 0) out vec4 outColor;

//Add normal mapping
//Add POM

vec4 aces(vec4 x) {
    vec4 tonemapped = clamp((x * (2.51f * x + 0.03f)) / (x * (2.43f * x + 0.59f) + 0.14f), 0.0f, 1.0f);
    return vec4(tonemapped.x, tonemapped.y, tonemapped.z, 1.0f);
}

void main() {
    outColor = aces(texture(diffuse, fragmentTextureCoordinates));
}
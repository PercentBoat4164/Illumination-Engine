#version 460
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
    float time;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inNormal;

layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 outPos;
layout(location = 3) out float time;


void main() {
    outPos = vec3(ubo.model * vec4(inPosition, 1.0f));
    gl_Position = ubo.proj * ubo.view * vec4(outPos, 1.0f);
    fragTexCoord = inTexCoord;
    time = fract(ubo.time);
}
#version 460
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform CameraData {
    mat4 viewModelMatrix;
    mat4 modelMatrix;
    mat4 projectionMatrix;
    mat4 normalMatrix;
    vec3 position;
    float time;
} cameraData;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec2 vertexTextureCoordinates;
layout(location = 3) in vec3 vertexNormal;

layout(location = 0) out vec2 fragmentTextureCoordinates;
layout(location = 1) out vec3 interpolatedNormal;
layout(location = 2) out vec3 fragmentPosition;

void main() {
    fragmentPosition = vec3(cameraData.modelMatrix * vec4(vertexPosition, 1.0f));
    gl_Position = cameraData.projectionMatrix * cameraData.viewModelMatrix * vec4(fragmentPosition, 1.0f);
    fragmentTextureCoordinates = vertexTextureCoordinates;
    interpolatedNormal = vec3(normalize(cameraData.normalMatrix * vec4(vertexNormal, 1.0f)));
}
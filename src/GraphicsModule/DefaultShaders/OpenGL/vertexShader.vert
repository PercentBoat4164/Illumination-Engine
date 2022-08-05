#version 460

uniform mat4 projectionViewModelMatrix;
uniform mat4 modelMatrix;
uniform mat4 normalMatrix;
uniform vec3 position;
uniform float time;

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec4 vertexColor;
layout(location = 2) in vec2 vertexTextureCoordinates;
layout(location = 3) in vec3 vertexNormal;
layout(location = 4) in vec3 vertexTangent;
layout(location = 5) in vec3 vertexBitangent;


layout(location = 0) out vec2 fragmentTextureCoordinates;
layout(location = 1) out vec3 interpolatedNormal;
layout(location = 2) out vec3 fragmentPosition;

void main() {
    fragmentPosition = vec3(modelMatrix * vec4(vertexPosition, 1.0f));
    gl_Position = projectionViewModelMatrix * vec4(fragmentPosition, 1.0f);
    fragmentTextureCoordinates = vertexTextureCoordinates;
    interpolatedNormal = vec3(normalize(normalMatrix * vec4(vertexNormal, 1.0f)));
}
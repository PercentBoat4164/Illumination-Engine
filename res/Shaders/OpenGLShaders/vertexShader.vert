#version 330 core

layout (location=0) in vec3 vertexPosition;
layout (location=1) in vec4 vertexColor;
layout (location=2) in vec2 vertexTextureCoordinates;
layout (location=3) in vec3 vertexNormal;

uniform mat4 projection;
uniform mat4 normalMatrix;
uniform mat4 modelView;
uniform mat4 model;

out vec2 fragmentTextureCoordinates;
out vec3 interpolatedNormal;
out vec3 fragmentPosition;

void main() {
    gl_Position = projection * modelView * vec4(vertexPosition, 1.0f);
    fragmentTextureCoordinates = vertexTextureCoordinates;
    fragmentPosition = vec3(model * vec4(vertexPosition, 1.0f));
    interpolatedNormal = normalize(mat3(normalMatrix) * vertexNormal);
}
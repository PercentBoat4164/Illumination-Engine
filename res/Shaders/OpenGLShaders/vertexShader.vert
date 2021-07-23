#version 330 core

layout (location=0) in vec3 vPos;
layout (location=1) in vec4 vColor;
layout (location=2) in vec2 vTexCoords;
layout (location=3) in vec3 vNormal;

uniform mat4 MVP;
uniform vec2 inResolution;

out vec4 color;
out vec2 texCoords;
out vec3 normal;
out vec2 fragResolution;

void main() {
    gl_Position = MVP * vec4(vPos, 1.0);
    color = vColor;
    texCoords = vTexCoords;
    normal = vNormal;
    fragResolution = inResolution;
}
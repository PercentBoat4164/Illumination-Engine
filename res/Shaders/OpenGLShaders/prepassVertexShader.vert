#version 330 core

layout (location=0) in vec3 vPos;
layout (location=1) in vec4 vColor;
layout (location=2) in vec2 vTexCoords;
layout (location=3) in vec3 vNormal;

uniform mat4 MVP;

out vec2 texCoords;

void main() {
    gl_Position = MVP * vec4(vPos, 1.0);
    texCoords = vTexCoords;
}
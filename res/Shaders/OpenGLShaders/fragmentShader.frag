#version 330 core

in vec4 color;
in vec2 texCoords;
in vec3 normal;

out vec4 fragColor;

uniform sampler2D diffuse;

void main() {
    fragColor = (texture(diffuse, texCoords) * (dot(normal, vec3(0, 0, 1) + 1)) / 2);
}
#version 330 core

in vec4 color;
in vec2 texCoords;
in vec3 normal;

out vec4 fragColor;

uniform sampler2D albedo;

void main() {
    fragColor = texture(albedo, texCoords) * color;
}
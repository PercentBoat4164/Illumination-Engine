#version 330 core

in vec2 texCoords;

out vec4 fragColor;

uniform sampler2D albedo;

void main() {
    if (texture(albedo, texCoords).w) { gl_FragDepth = 999999999999999; }
    fragColor = gl_FragDepth;
}
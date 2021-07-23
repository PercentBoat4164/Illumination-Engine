#version 330 core

in vec2 texCoords;

out vec4 fragColor;

uniform sampler2D diffuse;

void main() {
    if (texture(diffuse, texCoords).w < 1) { gl_FragDepth = 0; } else { gl_FragDepth = gl_FragCoord.z; }
}
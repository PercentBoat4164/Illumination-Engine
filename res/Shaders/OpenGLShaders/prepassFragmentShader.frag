#version 330 core

in vec2 texCoords;

out vec4 depth;

uniform sampler2D diffuse;

void main() {
    if (texture(diffuse, texCoords).w > 0) { gl_FragDepth = 2147483647; } else { gl_FragDepth = gl_FragCoord.z; }
    depth.x = gl_FragDepth;
}
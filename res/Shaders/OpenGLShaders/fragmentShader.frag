#version 330 core

in vec4 color;
in vec2 texCoords;
in vec3 normal;

out vec4 fragColor;

uniform sampler2D diffuse;
uniform sampler2D depth;

void main() {
    if (gl_FragDepth < texture(depth, gl_FragCoord.xy).r);
    fragColor = texture(diffuse, texCoords);
}
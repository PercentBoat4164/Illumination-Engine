#version 330 core

in vec4 color;
in vec2 texCoords;
in vec3 normal;
in vec2 fragResolution;

out vec4 fragColor;

uniform sampler2D diffuse;
uniform sampler2D depth;

void main() {
    if (gl_FragCoord.z > textureLod(depth, gl_FragCoord.xy / fragResolution, 0).x + .00001) { discard; } else { fragColor = texture(diffuse, texCoords); }
}
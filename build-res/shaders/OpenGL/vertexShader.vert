#version 140

uniform mat4 projectionViewModelMatrix;
uniform mat4 modelMatrix;
uniform mat4 normalMatrix;
uniform vec3 position;
uniform float time;

in vec3 vertexPosition;
in vec4 vertexColor;
in vec2 vertexTextureCoordinates;
in vec3 vertexNormal;
in vec3 vertexTangent;
in vec3 vertexBiTangent;


out vec2 fragmentTextureCoordinates;
out vec3 interpolatedNormal;
out vec3 fragmentPosition;

void main() {
    fragmentPosition = vec3(modelMatrix * vec4(vertexPosition, 1.0));
    gl_Position = projectionViewModelMatrix * vec4(fragmentPosition, 1.0);
    fragmentTextureCoordinates = vertexTextureCoordinates;
    interpolatedNormal = vec3(normalize(normalMatrix * vec4(vertexNormal, 1.0)));
}
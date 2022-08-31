#version 110

uniform mat4 projectionViewModelMatrix;
uniform mat4 modelMatrix;
uniform mat4 normalMatrix;
uniform vec3 position;
uniform float time;

attribute vec3 vertexPosition;
attribute vec4 vertexColor;
attribute vec2 vertexTextureCoordinates;
attribute vec3 vertexNormal;
attribute vec3 vertexTangent;
attribute vec3 vertexBiTangent;


varying vec2 fragmentTextureCoordinates;
varying vec3 interpolatedNormal;
varying vec3 fragmentPosition;

void main() {
    fragmentPosition = vec3(modelMatrix * vec4(vertexPosition, 1.0));
    gl_Position = projectionViewModelMatrix * vec4(fragmentPosition, 1.0);
    fragmentTextureCoordinates = vertexTextureCoordinates;
    interpolatedNormal = vec3(normalize(normalMatrix * vec4(vertexNormal, 1.0)));
}
#version 140
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable
#extension GL_ARB_explicit_attrib_location : enable

uniform mat4 projectionViewModelMatrix;
uniform mat4 modelMatrix;
uniform mat4 normalMatrix;
uniform vec3 position;
uniform float time;

layout(binding = 1) uniform sampler2D diffuseTexture;

layout(location = 0) in vec2 fragmentTextureCoordinates;
layout(location = 1) in vec3 interpolatedNormal;
layout(location = 2) in vec3 fragmentPosition;

layout(location = 0) out vec4 fragmentColor;

const vec3 lightPosition = vec3(0.0, 0.0, 2.0);
const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const float brightness = 10.0;
const float ambientStrength = 0.0;

const float PHI = 1.61803398874989484820459;

vec4 aces(vec4 x) {
    vec4 tonemapped = clamp((x * (2.51 * x + 0.03)) / (x * (2.43 * x + 0.59) + 0.14), 0.0, 1.0);
    return vec4(tonemapped.x, tonemapped.y, tonemapped.z, 1.0);
}

void main() {
    vec3 normalizedInterpolatedNormal = normalize(interpolatedNormal);
    float distanceFromFragmentToLight = length(fragmentPosition - lightPosition);
    float lightIntensityAfterAttenuation = 1 / (1 + distanceFromFragmentToLight + distanceFromFragmentToLight * distanceFromFragmentToLight) * brightness;
    vec3 lightDirection = normalize(lightPosition - fragmentPosition);
    vec3 diffuse = vec3(texture(diffuseTexture, fragmentTextureCoordinates)) * max(dot(normalizedInterpolatedNormal, lightDirection), 0.0f) * lightColor * lightIntensityAfterAttenuation;
    vec3 ambient = ambientStrength * lightColor;
    vec3 viewDirection = normalize(position - fragmentPosition);
//    vec3 specular = vec3(texture(specularTexture, fragmentTextureCoordinates)) * pow(max(dot(normalizedInterpolatedNormal, normalize(lightDirection + viewDirection)), 0.0f), 16.0f) * lightColor * lightIntensityAfterAttenuation;
//    fragmentColor = aces(vec4((ambient + diffuse + specular), 1.0f));
    fragmentColor = aces(vec4((ambient + diffuse), 1.0));
//    fragmentColor = aces(vec4(texture(diffuseTexture, fragmentTextureCoordinates)));
//    fragmentColor = vec4(1);
//    fragmentColor = vec4(fragmentTextureCoordinates, 0, 0);
//    fragmentColor = aces(vec4(gold_noise(vec3(gl_FragCoord), time), gold_noise(vec3(gl_FragCoord), time + 1), gold_noise(vec3(gl_FragCoord), time + 2), 1.0f));
}
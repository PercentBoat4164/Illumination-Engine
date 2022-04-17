#version 460
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform CameraData {
    mat4 viewModelMatrix;
    mat4 modelMatrix;
    mat4 projectionMatrix;
    mat4 normalMatrix;
    vec3 position;
    float time;
} cameraData;

layout(binding = 1) uniform sampler2D diffuseTexture;
//layout(binding = 2) uniform sampler2D specularTexture;

layout(location = 0) in vec2 fragmentTextureCoordinates;
layout(location = 1) in vec3 interpolatedNormal;
layout(location = 2) in vec3 fragmentPosition;

layout(location = 0) out vec4 fragmentColor;

const vec3 lightPosition = vec3(0.0f, 0.0f, 2.0f);
const vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
const float brightness = 10.0f;
const float ambientStrength = 0.0f;

vec4 aces(vec4 x) {
    vec4 tonemapped = clamp((x * (2.51f * x + 0.03f)) / (x * (2.43f * x + 0.59f) + 0.14f), 0.0f, 1.0f);
    return vec4(tonemapped.x, tonemapped.y, tonemapped.z, 1.0f);
}

void main() {
    vec3 normalizedInterpolatedNormal = normalize(interpolatedNormal);
    float distanceFromFragmentToLight = length(fragmentPosition - lightPosition);
    float lightIntensityAfterAttenuation = 1 / ( 1 + distanceFromFragmentToLight + distanceFromFragmentToLight * distanceFromFragmentToLight) * brightness;
    vec3 lightDirection = normalize(lightPosition - fragmentPosition);
    vec3 diffuse = vec3(texture(diffuseTexture, fragmentTextureCoordinates)) * max(dot(normalizedInterpolatedNormal, lightDirection), 0.0f) * lightColor * lightIntensityAfterAttenuation;
    vec3 ambient = ambientStrength * lightColor;
    vec3 viewDirection = normalize(cameraData.position - fragmentPosition);
//    vec3 specular = vec3(texture(specularTexture, fragmentTextureCoordinates)) * pow(max(dot(normalizedInterpolatedNormal, normalize(lightDirection + viewDirection)), 0.0f), 16.0f) * lightColor * lightIntensityAfterAttenuation;
//    fragmentColor = aces(vec4((ambient + diffuse + specular), 1.0f));
    fragmentColor = aces(vec4((ambient + diffuse), 1.0f));
}
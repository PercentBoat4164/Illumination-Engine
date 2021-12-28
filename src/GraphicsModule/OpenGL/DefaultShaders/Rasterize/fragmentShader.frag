#version 130

/*
 * This shader supports all GLSL version from 130 to 460 with no extensions.
 *
 * Example:
 * #version 120 // Not supported
 *
 * #version 130 // Supported - Lowest supported version
 *
 * #version 460 // Supported - Highest supported version
 */

in vec2 fragmentTextureCoordinates;
in vec3 interpolatedNormal;
in vec3 fragmentPosition;

uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform vec3 cameraPosition;

out vec4 fragmentColor;

const vec3 lightPosition = vec3(0.0f, 0.0f, 2.0f);
const vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
const float brightness = 25.0f;
const float ambientStrength = 0.01f;

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
    vec3 viewDirection = normalize(cameraPosition - fragmentPosition);
    vec3 specular = vec3(texture(specularTexture, fragmentTextureCoordinates)) * pow(max(dot(normalizedInterpolatedNormal, normalize(lightDirection + viewDirection)), 0.0f), 16.0f) * lightColor * lightIntensityAfterAttenuation;
    fragmentColor = aces(vec4((ambient + diffuse + specular), 1.0f));
}
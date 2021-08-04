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
const float brightness = 10.0f;
const float ambientStrength = 0.0f;

void main() {
    float distanceFromFragmentToLight = length(fragmentPosition - lightPosition);
    float lightIntensityAfterAttenuation = 1 / ( 1 + distanceFromFragmentToLight + distanceFromFragmentToLight * distanceFromFragmentToLight) * brightness;
    vec3 lightDirection = normalize(lightPosition - fragmentPosition);
    vec3 diffuse = max(dot(interpolatedNormal, lightDirection), 0.0f) * lightColor * lightIntensityAfterAttenuation;
    vec3 ambient = ambientStrength * lightColor;
    vec3 viewDirection = normalize(cameraPosition - fragmentPosition);
    vec3 reflectDirection = reflect(-lightDirection, interpolatedNormal);
    vec3 specular = vec3(texture(specularTexture, fragmentTextureCoordinates)) * pow(max(dot(viewDirection, reflectDirection), 0.0f), 32.0f) * lightColor * lightIntensityAfterAttenuation;
    fragmentColor = vec4((ambient + diffuse + specular), 1.0f) * texture(diffuseTexture, fragmentTextureCoordinates);
}
#version 110

uniform mat4 projectionViewModelMatrix;
uniform mat4 modelMatrix;
uniform mat4 normalMatrix;
uniform vec3 position;
uniform float time;

uniform sampler2D diffuseTexture;

in vec2 fragmentTextureCoordinates;
in vec3 interpolatedNormal;
in vec3 fragmentPosition;

varying out vec4 fragmentColor;

const vec3 lightPosition = vec3(0.0, 0.0, 2.0);
const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const float brightness = 10.0;
const float ambientStrength = 0.0;

vec4 aces(vec4 x) {
    vec4 tonemapped = clamp((x * (2.51 * x + 0.03)) / (x * (2.43 * x + 0.59) + 0.14), 0.0, 1.0);
    return vec4(tonemapped.x, tonemapped.y, tonemapped.z, 1.0);
}

void main() {
    vec3 normalizedInterpolatedNormal = normalize(interpolatedNormal);
    float distanceFromFragmentToLight = length(fragmentPosition - lightPosition);
    float lightIntensityAfterAttenuation = 1.0 / (1.0 + distanceFromFragmentToLight + distanceFromFragmentToLight * distanceFromFragmentToLight) * brightness;
    vec3 lightDirection = normalize(lightPosition - fragmentPosition);
    vec3 diffuse = vec3(texture2D(diffuseTexture, fragmentTextureCoordinates)) * max(dot(normalizedInterpolatedNormal, lightDirection), 0.0) * lightColor * lightIntensityAfterAttenuation;
//    vec3 diffuse = vec3(1.0) * max(dot(normalizedInterpolatedNormal, lightDirection), 0.0) * lightColor * lightIntensityAfterAttenuation;
    vec3 ambient = ambientStrength * lightColor;
    vec3 viewDirection = normalize(position - fragmentPosition);
//    vec3 specular = vec3(texture(specularTexture, fragmentTextureCoordinates)) * pow(max(dot(normalizedInterpolatedNormal, normalize(lightDirection + viewDirection)), 0.0), 16.0) * lightColor * lightIntensityAfterAttenuation;
//    fragmentColor = aces(vec4((ambient + diffuse + specular), 1.0));
//    fragmentColor = aces(vec4((ambient + diffuse), 1.0));
//    fragmentColor = aces(vec4(texture2D(diffuseTexture, fragmentTextureCoordinates)));
//    fragmentColor = aces(vec4(1));
    fragmentColor = vec4(normalizedInterpolatedNormal, 1.0);
}
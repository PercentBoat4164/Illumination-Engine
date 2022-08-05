#version 110

uniform mat4 projectionViewModelMatrix;
uniform mat4 modelMatrix;
uniform mat4 normalMatrix;
uniform vec3 position;
uniform float time;

uniform sampler2D diffuseTexture;

varying vec2 fragmentTextureCoordinates;
varying vec3 interpolatedNormal;
varying vec3 fragmentPosition;

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
    vec3 ambient = ambientStrength * lightColor;
    vec3 viewDirection = normalize(position - fragmentPosition);
    gl_FragColor = aces(vec4((ambient + diffuse), 1.0));
}
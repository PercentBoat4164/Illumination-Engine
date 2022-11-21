#version 460
#extension GL_ARB_separate_shader_objects: enable

layout (IE_ENGINE_DATA) uniform EngineData {
    uint frameNumber;
    float time;
} engine;

layout (IE_CAMERA_DATA) uniform CameraData {
    mat4 viewMatrix;
    vec3 viewDirection;
    vec3 position;
} camera;


layout (IE_PERSPECTIVE_DATA) uniform PerspectiveData {
    mat4 viewMatrix;
    vec3 viewDirection;
    vec3 position;
} perspective;


layout (IE_DIFFUSE_TEXTURE) uniform sampler2D diffuseTexture;
layout (IE_SPECULAR_TEXTURE) uniform sampler2D specularTexture;


layout (IE_OBJECT_DATA) uniform ObjectData {
    mat4 modelMatrix;
    mat3 normalMatrix;
} object;

layout (location = 0) in vec3 fragmentPosition;
layout (location = 1) in vec3 interpolatedColor;
layout (location = 2) in vec2 interpolatedTextureCoordinates;
layout (location = 3) in vec3 interpolatedNormal;
layout (location = 4) in vec3 interpolatedTangent;
layout (location = 5) in vec3 interpolatedBitangent;

layout (location = 0) out vec4 fragmentColor;

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
    float lightIntensityAfterAttenuation = 1 / (1 + distanceFromFragmentToLight + distanceFromFragmentToLight * distanceFromFragmentToLight) * brightness;
    vec3 lightDirection = normalize(lightPosition - fragmentPosition);
    vec3 diffuse = vec3(texture(diffuseTexture, interpolatedTextureCoordinates)) * max(dot(normalizedInterpolatedNormal, lightDirection), 0.0f) * lightColor * lightIntensityAfterAttenuation;
    vec3 ambient = ambientStrength * lightColor - interpolatedColor - interpolatedTangent - interpolatedBitangent;
    vec3 viewDirection = normalize(camera.position - fragmentPosition);
    vec3 specular = vec3(texture(specularTexture, interpolatedTextureCoordinates)) * pow(max(dot(normalizedInterpolatedNormal, normalize(lightDirection + viewDirection)), 0.0f), 16.0f) * lightColor * lightIntensityAfterAttenuation;
    fragmentColor = aces(vec4((ambient + diffuse + specular), 1.0f));
    //    fragmentColor = aces(vec4((ambient + diffuse), 1.0f));
    //    fragmentColor = aces(vec4(texture(diffuseTexture, fragmentTextureCoordinates)));
    //    fragmentColor = aces(vec4(1));
}
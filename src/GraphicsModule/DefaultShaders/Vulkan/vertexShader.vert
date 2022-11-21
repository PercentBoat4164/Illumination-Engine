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

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexColor;
layout (location = 2) in vec2 vertexTextureCoordinates;
layout (location = 3) in vec3 vertexNormal;
layout (location = 4) in vec3 vertexTangent;
layout (location = 5) in vec3 vertexBitangent;


layout (location = 0) out vec3 fragmentPosition;
layout (location = 1) out vec3 interpolatedColor;
layout (location = 2) out vec2 interpolatedTextureCoordinates;
layout (location = 3) out vec3 interpolatedNormal;
layout (location = 4) out vec3 interpolatedTangent;
layout (location = 5) out vec3 interpolatedBitangent;

void main() {
    // This is incorrect maths! Not all information is available at the moment, so this is just a placeholder.
    fragmentPosition = vec3(camera.viewMatrix * vec4(vertexPosition, 1.0f));
    gl_Position = camera.viewMatrix * vec4(fragmentPosition, 1.0f);
    interpolatedTextureCoordinates = vertexTextureCoordinates;
    interpolatedNormal = vec3(normalize(object.normalMatrix * vertexNormal));
    interpolatedColor = vertexColor;
    interpolatedTangent = vertexTangent;
    interpolatedBitangent = vertexBitangent;
}
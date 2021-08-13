#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_ray_query : enable

const float PHI = 1.61803398874989484820459;

//LIGHT DATA
const vec3 position = vec3(0, 0, 2);
const float radius = 0.06f;
vec3 color = vec3(0xe2, 0x58, 0x22) / 255;

//SAMPLE COUNT
const int samples = 1;

layout(binding = 1) uniform sampler2D diffuse;
layout(binding = 2) uniform accelerationStructureEXT topLevelAccelerationStructure;

layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragmentPositionInWorldSpace;
layout(location = 3) in float time;


layout(location = 0) out vec4 outColor;

//Add normal mapping
//Add POM

float gold_noise(in vec2 xy, in float seed) {
    return max(fract(tan(distance(xy * PHI, xy) * seed) * xy.x), 0.01f);
}

float hash(float p) { p = fract(p * 0.011); p *= p + 7.5; p *= p + p; return fract(p); }

float noise(float x) {
    float i = floor(x);
    float f = fract(x);
    float u = f * f * (3.0 - 2.0 * f);
    return mix(hash(i), hash(i + 1.0), u);
}

float fbm(float x) {
    float v = 0.0f;
    float a = 0.5f;
    float j = 0.0f;
    float f = 0.0f;
    for (int i = 0; i < 5; ++i) {
        j = floor(x);
        f = fract(x);
        v += a * mix(hash(j), hash(j + 1.0f), f * f * (3.0f - 2.0f * f));
        x = x * 2.0f + 100;
        a *= 0.5f;
    }
    return v;
}

vec4 aces(vec4 x) {
    vec4 tonemapped = clamp((x * (2.51f * x + 0.03f)) / (x * (2.43f * x + 0.59f) + 0.14f), 0.0f, 1.0f);
    return vec4(tonemapped.x, tonemapped.y, tonemapped.z, 1.0f);
}

void main() {
    float fbmResultAtThisTime = fbm(time);
    float fbmResultAtDifferentTime = fbm(time + 100000);
    float brightness = 40 * fbmResultAtThisTime;
    color *= vec3(max(fbmResultAtDifferentTime * 2, 1.0), 1, 1);
    outColor = vec4(0);
    float distanceFromFragmentToLight = distance(fragmentPositionInWorldSpace, vec3(0, 0, 2));
    float lightIntensityAfterAttenuation = 1 / ( 1 + distanceFromFragmentToLight + distanceFromFragmentToLight * distanceFromFragmentToLight) * brightness;
    rayQueryEXT rayQuery;
    for (int i = 0; i < samples; ++i) {
        vec3 randomPointOnUnitSphere = normalize(vec3(gold_noise(gl_FragCoord.xy, time + 1 + i), gold_noise(gl_FragCoord.xy, time + 2 + i), gold_noise(gl_FragCoord.xy, time + 3 + i)));
        rayQueryInitializeEXT(rayQuery, topLevelAccelerationStructure, gl_RayFlagsTerminateOnFirstHitEXT, 0xFF, fragmentPositionInWorldSpace, 0.001, -normalize(fragmentPositionInWorldSpace - (position + randomPointOnUnitSphere * radius)), distanceFromFragmentToLight);
        while (rayQueryProceedEXT(rayQuery)) { }
        if (rayQueryGetIntersectionTypeEXT(rayQuery, true) != gl_RayQueryCommittedIntersectionTriangleEXT) { outColor += texture(diffuse, fragTexCoord) * vec4(color, 0) * lightIntensityAfterAttenuation; }
    }
    outColor /= samples;
    outColor = aces(outColor);
}
#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_ray_query : enable

const float PHI = 1.61803398874989484820459;

//LIGHT DATA
const vec3 position = vec3(0, 0, 2);
const float radius = 0.06f;
const float brightness = 10.0f;

//SAMPLE COUNT
const int samples = 1;

layout(binding = 1) uniform sampler2D diffuse;
layout(binding = 2) uniform accelerationStructureEXT topLevelAS;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragmentPositionInWorldSpace;
layout(location = 3) in float time;


layout(location = 0) out vec4 outColor;

//Add normal mapping
//Add POM

float noise(in vec2 xy, in float seed) {
    return max(fract(tan(distance(xy * PHI, xy) * seed) * xy.x), 0.01f);
}

void main() {
    outColor = vec4(0);
    float distanceFromFragmentToLight = distance(fragmentPositionInWorldSpace, vec3(0, 0, 2));
    float lightIntensityAfterAttenuation = 1 / ( 1 + distanceFromFragmentToLight + distanceFromFragmentToLight * distanceFromFragmentToLight) * brightness;
    rayQueryEXT rayQuery;
    for (int i = 0; i < samples; ++i) {
        vec3 randomPointOnUnitSphere = normalize(vec3(noise(gl_FragCoord.xy, time + 1 + i), noise(gl_FragCoord.xy, time + 2 + i), noise(gl_FragCoord.xy, time + 3 + i)));
        rayQueryInitializeEXT(rayQuery, topLevelAS, gl_RayFlagsTerminateOnFirstHitEXT, 0xFF, fragmentPositionInWorldSpace, 0.001, -normalize(fragmentPositionInWorldSpace - (position + randomPointOnUnitSphere * radius)), distanceFromFragmentToLight);

        // Start the ray traversal, rayQueryProceedEXT returns false if the traversal is complete
        while (rayQueryProceedEXT(rayQuery)) { }

        // If the intersection has hit a triangle, the fragment is shadowed
        if (rayQueryGetIntersectionTypeEXT(rayQuery, true) != gl_RayQueryCommittedIntersectionTriangleEXT) { outColor += texture(diffuse, fragTexCoord) * lightIntensityAfterAttenuation; }
    }
    outColor /= samples;
}
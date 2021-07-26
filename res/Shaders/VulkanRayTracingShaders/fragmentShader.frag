#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_ray_query : enable

layout(binding = 1) uniform sampler2D diffuse;
layout(binding = 2) uniform accelerationStructureEXT topLevelAS;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 worldPosition;

layout(location = 0) out vec4 outColor;

//Add normal mapping
//Add POM

void main() {
    outColor = texture(diffuse, fragTexCoord) * 0.1f;
    rayQueryEXT rayQuery;
    rayQueryInitializeEXT(rayQuery, topLevelAS, gl_RayFlagsTerminateOnFirstHitEXT, 0xFF, worldPosition, 0.001, vec3(0, 0, 1), 1000.0);

    // Start the ray traversal, rayQueryProceedEXT returns false if the traversal is complete
    while (rayQueryProceedEXT(rayQuery)) {  }

    // If the intersection has hit a triangle, the fragment is shadowed
    if (rayQueryGetIntersectionTypeEXT(rayQuery, true) != gl_RayQueryCommittedIntersectionTriangleEXT ) { outColor *= 10; }
}
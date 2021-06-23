#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) rayPayloadInEXT vec3 hitValue;
layout(location = 0) callableDataEXT vec3 outColor;
hitAttributeEXT vec3 attribs;

layout(binding = 0, set = 0) uniform accelerationStructureEXT topLevelAS;
//layout(binding = 3, set = 0) buffer Vertices { vec4 v[]; } vertices;
//layout(binding = 4, set = 0) buffer Indices { uint i[]; } indices;

struct Vertex {
    vec3 pos;
    vec4 color;
    vec2 UV;
    vec3 normal;
};

//Vertex unpack(uint index) {
    //create a vertex from vertex at index
    //vec4 dataBlock0 = vertices.v[4 * index + 0];
    //vec4 dataBlock1 = vertices.v[4 * index + 1];
    //vec4 dataBlock2 = vertices.v[4 * index + 2];
    //Vertex vertex;
    //vertex.pos = dataBlock0.xyz;
    //vertex.color = vec4(dataBlock0.w, dataBlock1.xyz);
    //vertex.UV = vec2(dataBlock1.w, dataBlock2.x);
    //vertex.normal = dataBlock2.yzw;
    //return vertex;
//}

void main() {
    //const ivec3 index = ivec3(indices.i[3 * gl_PrimitiveID], indices.i[3 * gl_PrimitiveID + 1], indices.i[3 * gl_PrimitiveID + 2]);

    //const Vertex vertex0 = unpack(index.x);
    //const Vertex vertex1 = unpack(index.y);
    //const Vertex vertex2 = unpack(index.z);

    const vec3 barycentricCoords = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
    //vec3 normal = normalize(vertex0.normal * barycentricCoords.x + vertex1.normal * barycentricCoords.y + vertex2.normal * barycentricCoords.z);
    //vec3 worldPos = vertex0.pos * barycentricCoords.x + vertex1.pos * barycentricCoords.y + vertex2.pos * barycentricCoords.z;

    hitValue = vec3(gl_PrimitiveID, 0.0, 0.0);
    //hitValue = barycentricCoords;

    //executeCallableEXT(gl_GeometryIndexEXT, 0);

    //hitValue = outColor;
}

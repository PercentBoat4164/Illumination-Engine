#version 460
#extension GL_EXT_ray_tracing : require

layout(location = 0) rayPayloadInEXT vec3 hitValue;

void main()
{
    hitValue = vec3(gl_WorldRayDirectionEXT.xy, 0);
}
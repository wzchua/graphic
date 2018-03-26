#version 450
#extension GL_ARB_bindless_texture : require

in vec3 wcPosition;   // Fragment's 3D position in world space.
in vec3 wcNormal;

layout(binding = 0, RGBA8) uniform image2D worldPositionMap;
layout(binding = 1, RGBA8) uniform image2D worldNormalMap;

void main()
{
    ivec2 xy = ivec2(gl_FragCoord.xy);
    imageStore(worldPositionMap, xy, vec4(wcPosition, 1.0f));
    imageStore(worldNormalMap, xy, vec4(normalize(wcNormal), 1.0f));
}
#version 450
#extension GL_ARB_bindless_texture : require

in vec3 wcPosition;   // Fragment's 3D position in world space.
in vec3 wcNormal;

layout(binding = 3, std140) uniform VoxelizeMatrixBlock {
    mat4 WorldToVoxelMat;
    mat4 ViewProjMatrixXY; 
};
layout(binding = 0, rgb10_a2) uniform image2D voxelPositionMap;
layout(binding = 1, rgb10_a2) uniform image2D worldNormalMap;
const float voxelDim = 512.0f;
void main()
{
    ivec2 xy = ivec2(gl_FragCoord.xy);
    vec4 voxelPos = WorldToVoxelMat * vec4(wcPosition, 1.0f);
    voxelPos.xyz /= voxelDim;
    imageStore(voxelPositionMap, xy, voxelPos);
    imageStore(worldNormalMap, xy, vec4(normalize(wcNormal), 1.0f) * 0.5f + 0.5f);
}
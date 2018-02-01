#version 450
#extension GL_ARB_bindless_texture : require

layout (location = 0) in vec3 vPosition;  // Vertex position in object space.
layout (location = 1) in vec3 vNormal;    // Vertex normal in object space.
layout (location = 2) in vec2 vTexCoord;    // Vertex normal in object space.

layout(binding = 0, std140) uniform VoxelizeMatrixBlock {
    mat4 ModelMatrix;
    mat4 ViewProjMatrixXY; 
    mat4 ViewProjMatrixZY; 
    mat4 ViewProjMatrixXZ;
};

out vec3 geomwcPosition;   // Vertex position in scaled world space.
out vec3 geomwcNormal;     // Vertex normal in world space.
out vec2 geomTexCoord;

void main()
{
    vec4 wcPosition4 = ModelMatrix * vec4(vPosition, 1.0f);
    geomwcPosition = vec3( wcPosition4 ) / wcPosition4.w;
    geomwcNormal = vNormal;
    geomTexCoord = vTexCoord;
    gl_Position = ViewProjMatrixXY * vec4(vPosition, 1.0f);
}
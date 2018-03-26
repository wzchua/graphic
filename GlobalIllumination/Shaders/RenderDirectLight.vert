#version 450
#extension GL_ARB_bindless_texture : require

layout (location = 0) in vec3 vPosition;  // Vertex position in object space.
layout (location = 1) in vec3 vNormal;    // Vertex normal in object space.
layout (location = 2) in vec2 texCoord;    // Vertex normal in object space.

layout(binding = 0, std140) uniform MatrixBlock {
    mat4 ModelViewMatrix;     // ModelView matrix.
    mat4 ModelViewProjMatrix; // ModelView matrix * Projection matrix.
    mat3 NormalMatrix;        // For transforming object-space direction 
};                                    //   vector to eye space.

layout (location = 0) out vec3 wcPosition;   // Vertex position in eye space.
layout (location = 1) out vec3 ecNormal;     // Vertex normal in eye space.
layout (location = 2) out vec2 fTexCoord;

void main()
{
    
    wcPosition = vPosition;
    ecNormal = normalize( NormalMatrix * vNormal );
    fTexCoord = texCoord;
    gl_Position = ModelViewProjMatrix * vec4(vPosition, 1.0);
}
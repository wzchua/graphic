#version 450
#extension GL_ARB_bindless_texture : require

layout (location = 0) in vec3 vPosition;  // Vertex position in object space.
layout (location = 1) in vec3 vNormal;    // Vertex normal in object space.
layout (location = 2) in vec2 texCoord;    // Vertex normal in object space.
layout (location = 3) in sampler2D vTexAmbient;
layout (location = 4) in sampler2D vTexDiffuse;
layout (location = 5) in sampler2D vTexAlpha;
layout (location = 6) in sampler2D vTexHeight;
layout (location = 7) in int vUseBumpMap;
layout (location = 8) in vec3 vAmbient;
layout (location = 9) in vec3 vDiffuse;
layout (location = 10) in vec3 vSpecular;
layout (location = 11) in float vShininess;

layout(binding = 0, std140) uniform MatrixBlock {
    mat4 ModelViewMatrix;     // ModelView matrix.
    mat4 ModelViewProjMatrix; // ModelView matrix * Projection matrix.
    mat3 NormalMatrix;        // For transforming object-space direction 
};                                    //   vector to eye space.

layout (location = 0) out vec3 ecPosition;   // Vertex position in eye space.
layout (location = 1) out vec3 ecNormal;     // Vertex normal in eye space.
layout (location = 2) out vec2 fTexCoord;
/*
layout (location = 3) flat out sampler2D texAmbient;
layout (location = 4) flat out sampler2D texDiffuse;
layout (location = 5) flat out sampler2D texAlpha;
layout (location = 6) flat out sampler2D texHeight;
layout (location = 7) flat out int useBumpMap;
layout (location = 8) flat out vec3 ambient;
layout (location = 9) flat out vec3 diffuse;
layout (location = 10) flat out vec3 specular;
layout (location = 11) flat out float shininess;*/

void main()
{
    /*
    texAmbient = vTexAmbient;
    texDiffuse = vTexDiffuse;
    texAlpha = vTexAlpha;
    texHeight = vTexHeight;
    useBumpMap = vUseBumpMap;
    ambient = vAmbient;
    diffuse = vDiffuse;
    specular = vSpecular;
    shininess = vShininess;*/
    
    vec4 ecPosition4 = ModelViewMatrix * vec4(vPosition, 1.0);
    ecPosition = vec3( ecPosition4 ) / ecPosition4.w;
    ecNormal = normalize( NormalMatrix * vNormal );
    fTexCoord = texCoord;
    gl_Position = ModelViewProjMatrix * vec4(vPosition, 1.0);
}
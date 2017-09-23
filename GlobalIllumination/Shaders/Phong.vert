#version 450
layout (location = 0) in vec4 vPosition;  // Vertex position in object space.
layout (location = 1) in vec3 vNormal;    // Vertex normal in object space.
layout (location = 2) in vec2 texCoord;    // Vertex normal in object space.

uniform mat4 ModelViewMatrix;     // ModelView matrix.
uniform mat4 ModelViewProjMatrix; // ModelView matrix * Projection matrix.
uniform mat3 NormalMatrix;        // For transforming object-space direction 
                                  //   vector to eye space.

out vec3 ecPosition;   // Vertex position in eye space.
out vec3 ecNormal;     // Vertex normal in eye space.
out vec2 fTexCoord;

void main()
{
    vec4 ecPosition4 = ModelViewMatrix * vPosition;
    ecPosition = vec3( ecPosition4 ) / ecPosition4.w;
    ecNormal = normalize( NormalMatrix * vNormal );
    fTexCoord = texCoord;
    gl_Position = ModelViewProjMatrix * vPosition;
}
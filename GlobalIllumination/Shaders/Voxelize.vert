#version 450
layout (location = 0) in vec4 vPosition;  // Vertex position in object space.
layout (location = 1) in vec3 vNormal;    // Vertex normal in object space.
layout (location = 2) in vec2 texCoord;    // Vertex normal in object space.

uniform mat4 ModelMatrix;     // ModelView matrix.
uniform mat4 ModelViewMatrix;     // ModelView matrix.
uniform mat4 ModelViewProjMatrix; // ModelView matrix * Projection matrix.

out vec3 vwcPosition;   // Vertex position in scaled world space.
out vec3 vwcNormal;     // Vertex normal in world space.
out vec2 vTexCoord;

void main()
{
    vec4 wcPosition4 = ModelMatrix * vPosition;
    vwcPosition = vec3( wcPosition4 ) / wcPosition4.w;
    vwcNormal = vNormal;
    vTexCoord = texCoord;
    gl_Position = ModelViewProjMatrix * vPosition;
}
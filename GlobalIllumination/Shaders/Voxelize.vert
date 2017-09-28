#version 450
layout (location = 0) in vec4 vPosition;  // Vertex position in object space.
layout (location = 1) in vec3 vNormal;    // Vertex normal in object space.
layout (location = 2) in vec2 texCoord;    // Vertex normal in object space.

uniform mat4 ModelMatrix;     // ModelView matrix.
uniform mat4 ModelViewMatrix;     // ModelView matrix.
uniform mat4 ModelViewProjMatrix; // ModelView matrix * Projection matrix.

out vec3 wcPosition;   // Vertex position in scaled world space.
out vec3 wcNormal;     // Vertex normal in world space.
out vec2 fTexCoord;

void main()
{
    vec4 wcPosition4 = ModelMatrix * vPosition;
    wcPosition = vec3( wcPosition4 ) / wcPosition4.w;
    wcNormal = vNormal;
    fTexCoord = texCoord;
    gl_Position = ModelViewProjMatrix * vPosition;
}
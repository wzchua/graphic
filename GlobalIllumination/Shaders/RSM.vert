#version 450
layout (location = 0) in vec3 vPosition;  // Vertex position in object space.

uniform mat4 ModelMatrix;
uniform mat4 ProjViewModelMatrix; 

out vec3 wcPosition;   // Vertex position in scaled world space.

void main()
{
    vec4 wcPosition4 = ModelMatrix * vec4(vPosition, 1.0f);
    vwcPosition = vec3( wcPosition4 ) / wcPosition4.w;
    gl_Position = ProjViewModelMatrix * vec4(vPosition, 1.0f);
}
#version 450
layout (location = 0) in vec4 vPosition;  // Vertex position in eye space.

uniform mat4 ModelMatrix;     // ModelView matrix.
uniform mat4 InverseViewMatrix;

out vec3 wcPosition;   // Vertex position in voxel world space.

void main()
{
    vec4 wcPosition4 = InverseViewMatrix * vPosition; //move quad to voxel world space
    wcPosition = vec3( wcPosition4 ) / wcPosition4.w;

    
    gl_Position = vPosition; //quad does not move for the pipeline
}
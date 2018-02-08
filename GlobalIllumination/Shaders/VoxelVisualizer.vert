#version 450
layout (location = 0) in vec4 vPosition;  // Vertex position in voxel space.

void main()
{
    gl_Position = vPosition;
}
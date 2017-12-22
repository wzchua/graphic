#version 450
layout (location = 0) in vec3 vPosition;  // Vertex position in voxel space.

void main()
{
    gl_Position = vec4(vPosition, 1.0f);
}
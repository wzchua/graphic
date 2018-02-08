#version 450 core
flat in ivec3 voxelPos;

layout(binding = 4, RGBA8) uniform image3D colorBrick;

layout (location = 0) out vec4 FragColor;
void main() {
    FragColor = imageLoad(colorBrick, voxelPos);
}
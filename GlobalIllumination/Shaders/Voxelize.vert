#version 450
#extension GL_ARB_shader_atomic_counter_ops : require
#extension GL_ARB_bindless_texture : require

layout (location = 0) in vec3 vPosition;  // Vertex position in object space.
layout (location = 1) in vec3 vNormal;    // Vertex normal in object space.
layout (location = 2) in vec2 vTexCoord;    // Vertex normal in object space.

layout(binding = 0, std140) uniform VoxelizeMatrixBlock {
    mat4 WorldToVoxelMat;
    mat4 ViewProjMatrixXY; 
    mat4 ViewProjMatrixZY; 
    mat4 ViewProjMatrixXZ;
};
layout(binding = 7, std140) uniform LimitsUniformBlock {
    uint maxNoOfFragments;
    uint maxNoOfNodes;
    uint maxNoOfBricks;
    uint maxNoOfLogs;
};

layout(binding = 1) coherent buffer CounterBlock {
    uint fragmentCounter;
    uint nodeCounter;
    uint brickCounter;
    uint leafCounter;
    uint logCounter;
    uint noOfFragments;
};
struct LogStruct {
    vec4 position;
    vec4 color;
    uint nodeIndex;
    uint brickPtr;
    uint index1;
    uint index2;
};
layout(binding = 7) coherent buffer LogBlock {
    LogStruct logList[];
};

void logFragment(vec4 pos, vec4 color, uint nodeIndex, uint brickPtr, uint index1, uint index2) {
    uint index = atomicAdd(logCounter, 1);
    if(index < maxNoOfLogs) {        
        logList[index].position = pos;
        logList[index].color = color;
        logList[index].nodeIndex = nodeIndex;
        logList[index].brickPtr = brickPtr;
        logList[index].index1 = index1;
        logList[index].index2 = index2;
    } else {
        atomicAdd(logCounter, uint(-1));
    }
}

out vec3 geomwcPosition;   // Vertex position in scaled world space.
out vec3 geomwcNormal;     // Vertex normal in world space.
out vec2 geomTexCoord;

void main()
{
    vec4 wcPosition4 = WorldToVoxelMat * vec4(vPosition, 1.0f);
    geomwcPosition = vec3( wcPosition4 ) / wcPosition4.w;
    geomwcNormal = vNormal; //scale is done 1:1:1 ratio
    geomTexCoord = vTexCoord;
    gl_Position = ViewProjMatrixXY * WorldToVoxelMat * vec4(vPosition, 1.0f);
}
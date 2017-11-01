#version 450

in vec3 wcPosition;   // Vertex position in scaled world space.

layout(binding = 6) uniform atomic_uint lightBrickPtr;
layout(binding = 7) uniform atomic_uint logPtr;

layout(binding = 0, RGBA8) uniform image2D lightmapPosition;
layout(binding = 1, r32f) uniform image2D lightmapDepth;
layout(binding = 2, r32ui) uniform uimage2D lightmapLeafNode;

struct NodeStruct {
    uint parentPtr;
    uint selfPtr;
    uint childPtr;
    uint childBit;
    uint modelBrickPtr;
    uint lightBit;
    uint lightBrickPtr;
};

layout(binding = 2) volatile buffer NodeBlock {
    NodeStruct node[];
};

uniform uint maxNoOfLogs;
struct LogStruct {
    vec4 position;
    vec4 color;
    uint nodeIndex;
    uint brickPtr;
    uint index1;
    uint index2;
};

layout(binding = 7) volatile buffer LogBlock {
    LogStruct logList[];
};

void logFragment(vec4 pos, vec4 color, uint nodeIndex, uint brickPtr, uint index1, uint index2) {
    uint index = atomicCounterIncrement(logPtr);
    if(index < maxNoOfLogs) {        
        logList[index].position = pos;
        logList[index].color = color;
        logList[index].nodeIndex = nodeIndex;
        logList[index].brickPtr = brickPtr;
        logList[index].index1 = index1;
        logList[index].index2 = index2;
    } else {
        atomicCounterDecrement(logPtr);
    }
}

uint getPtrOffset(ivec3 frameOffset) {
    return min(frameOffset.x, 1) * 1 
    + min(frameOffset.y, 1) * 2 + min(frameOffset.z, 1) * 4;
}

uniform float levels[10] = float[10](0.001953125f, 0.00390625f, 0.0078125f,
                                0.015625f, 0.03125f, 0.0625f,
                                0.125f, 0.25f, 0.5f, 1.0f);

uniform int lowestLevel = 9;
ivec3[10] computeLevelOffset() {
    ivec3 frameOffset = ivec3(0);
    ivec3 prevFrameOffset = ivec3(0);
    ivec3 levelOffsets[10];
    vec3 position = vec3(frag[gl_GlobalInvocationID.x].position);
    for(int i = 0; i < 10; i++) {
        prevFrameOffset = frameOffset * 2;
        frameOffset = ivec3(position * levels[i]);
        levelOffsets[i] = frameOffset - prevFrameOffset;
    }
    return levelOffsets;
}

void main() {        
    imageStore(lightmapPosition, ivec2(gl_FragCoord.xy), vec4(wcPosition, 1.0f));
    imageStore(lightmapDepth, ivec2(gl_FragCoord.xy), vec4(gl_FragCoord.z));
    
    // start in root node
    int currentLevel = 0;

    while(currentLevel < lowestLevel - 1) {
        // move to next node  
        currentLevel++;
        uint intermediateChildOffset = getPtrOffset(levelOffsets[currentLevel]);
        atomicOr(node[nodeIndex].lightBit, 1 << intermediateChildOffset);
        nodeIndex = node[nodeIndex].childPtr + intermediateChildOffset;
    }
    ivec3 innerFrameOffset = levelOffsets[lowestLevel];
    uint leafOffset = getPtrOffset(innerFrameOffset);
    atomicOr(node[nodeIndex].lightBit, 1 << leafOffset);

    uint leafState = atomicCompSwap(node[nodeIndex].lightBrickPtr, 0, ISPROCESS); 
    //initialize light brick if not initialized   
    if(leafState == 0) {
        node[nodeIndex].lightBrickPtr = atomicCounterIncrement(lightBrickPtr);
    }
    imageStore(lightmapLeafNode, ivec2(gl_FragCoord.xy), vec4((nodeIndex << 3) & leafOffset));
}
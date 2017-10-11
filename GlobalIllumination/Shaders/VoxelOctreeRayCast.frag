#version 450

#define M_PI 3.1415926535897932384626433832795

in vec3 wcPosition;   // Vertex position in scaled world space.

uniform int height;
uniform int width;
uniform vec3 camPosition;
uniform vec3 camForward;
uniform vec3 camUp;

layout(binding = 4, RGBA8) uniform image3D colorBrick;
layout(binding = 5, RGBA8) uniform image3D normalBrick;

uniform uint INVALID = 0 - 1;
uniform int lowestLevel = 9;
uniform float levels[10] = float[10](0.001953125f, 0.00390625f, 0.0078125f,
                                0.015625f, 0.03125f, 0.0625f,
                                0.125f, 0.25f, 0.5f, 1.0f);

struct NodeStruct {
    uint parentPtr;
    uint selfPtr;
    uint childPtr;
    uint childBit;
    uint modelBrickPtr;
    uint lightBrickPtr;
};

layout(binding = 2) volatile buffer NodeBlock {
    NodeStruct node[];
};



ivec3[10] computeLevelOffset(vec3 pointPosition) {
    ivec3 frameOffset = ivec3(0);
    ivec3 prevFrameOffset = ivec3(0);
    ivec3 levelOffsets[10];
    vec3 position = vec3(pointPosition);
    for(int i = 0; i < 10; i++) {
        prevFrameOffset = frameOffset * 2;
        frameOffset = ivec3(position * levels[i]);
        levelOffsets[i] = frameOffset - prevFrameOffset;
    }
    return levelOffsets;
}

uint getPtrOffset(ivec3 frameOffset) {
    return min(frameOffset.x, 1) * 1 
    + min(frameOffset.y, 1) * 2 + min(frameOffset.z, 1) * 4;
}
uint enterNode(uint nodeIndex, uint offset) {
    uint childIndex = node[nodeIndex].childPtr;
    if(childIndex == 0) {
        return INVALID;
    }
    return childIndex + offset;
}

uint getLeafAtPosition(vec3 position, out ivec3 brickOffset) {
    // start in root node
    uint nodeIndex = 0;
    int currentLevel = 0;
    ivec3 offsets[10] = computeLevelOffset(position);
    brickOffset = ivec3(0);
    while(currentLevel < lowestLevel - 1) {
        // move to next node  
        currentLevel++;
        nodeIndex = enterNode(nodeIndex, getPtrOffset(offsets[currentLevel]));
        if(nodeIndex == INVALID || nodeIndex == 0){
            return INVALID;
        }
    }
    uint leafOffset = getPtrOffset(offsets[lowestLevel]);
    uint childBit = node[nodeIndex].childBit;
    if(((childBit >> leafOffset) & 1) == 0) {
        return INVALID;
    }
    uint brickPtr = node[nodeIndex].modelBrickPtr;
    uint bx = (brickPtr & 0x1FF) * 2;
    uint by = (brickPtr >> 9) * 2;
    ivec3 innerFrameOffset = offsets[lowestLevel];
    brickOffset = ivec3(bx + innerFrameOffset.x, by + innerFrameOffset.y, innerFrameOffset.z);
    return nodeIndex;
}

bool isRayInCubeSpace(vec3 rayPosition) {
    return rayPosition.x >= 0.0f && rayPosition.x <=512.0f
        && rayPosition.y >= 0.0f && rayPosition.y <=512.0f
        && rayPosition.z >= 0.0f && rayPosition.z <=512.0f;
}
layout(binding = 7) uniform atomic_uint logPtr;

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
    if(index < 500) {        
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

layout (location = 0) out vec4 FragColor;
void main() {    
    
    vec3 rOrigin = camPosition;
    float x = gl_FragCoord.x;
    float y = gl_FragCoord.y;
    vec3 viewPlaneCenter = camPosition + camForward * 0.5;
    vec3 U = normalize(cross(camForward, camUp));
    vec3 V = normalize(cross(U, camForward));
    float imageAspectRatio = width / float(height); // assuming width > height 
    float halfViewWidth = tan(60.0f * M_PI / 360) * 0.5;
    float halfViewHeight = halfViewWidth * imageAspectRatio;
    vec3 viewBottomLeft = viewPlaneCenter - U * halfViewWidth - V * halfViewHeight;
    vec3 px = x * (2 * U * halfViewWidth)/width; 
    vec3 py = y * (2 * V * halfViewHeight)/height;
    vec3 dir = normalize(viewBottomLeft + px + py - rOrigin);
    if( x < 1 && y < 1) {
        logFragment(vec4(px, 0f), vec4(dir, imageAspectRatio), height, width, 0, 0);
    }
    uint leafIndex = 0;
    vec3 rayPosition = rOrigin;
    ivec3 brickOffset;
    bool isRayInCube = true;
    //ray march
    do {
        
        if( x < 1 && y < 1) {
            logFragment(vec4(rayPosition, x), vec4(dir, y), leafIndex, 0, 0, isRayInCube ? 1 : 0);
        }
        rayPosition += dir;
        isRayInCube = isRayInCubeSpace(rayPosition);
        leafIndex = getLeafAtPosition(rayPosition, brickOffset);
    } while(leafIndex == INVALID && isRayInCube);

    if( x < 1 && y < 1) {
        logFragment(vec4(rayPosition, x), vec4(dir, y), leafIndex, uint(brickOffset.x), uint(brickOffset.y), isRayInCube ? 1 : 0);
    }
    if(isRayInCube) {
        FragColor = imageLoad(colorBrick, brickOffset); 
        
        if( x < 1 && y < 1) {
            logFragment(vec4(rayPosition, x), FragColor, leafIndex, uint(brickOffset.x), uint(brickOffset.y), isRayInCube ? 1 : 0);
        }

    } else {
        discard;
    }
}
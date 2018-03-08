#version 450

#define M_PI 3.1415926535897932384626433832795

in vec3 wcPosition;   // Vertex position in scaled world space.

layout(binding = 0) uniform RayCastBlock {
    mat4 ViewToVoxelMat;
    vec4 camPosition;
    vec4 camForward;
    vec4 camUp;
    int height;
    int width;
};
layout(binding = 7, std140) uniform LimitsUniformBlock {
    uint maxNoOfFragments;
    uint maxNoOfNodes;
    uint maxNoOfBricks;
    uint maxNoOfLogs;
};

layout(binding = 4, RGBA8) uniform image3D colorBrick;
//layout(binding = 5, RGBA8) uniform image3D normalBrick;

uniform uint INVALID = 0 - 1;
const float levels[9] = float[9](0.00390625f, 0.0078125f,
                                0.015625f, 0.03125f, 0.0625f,
                                0.125f, 0.25f, 0.5f, 1.0f);

layout(binding = 1) coherent buffer CounterBlock {
    uint fragmentCounter;
    uint nodeCounter;
    uint brickCounter;
    uint leafCounter;
    uint logCounter;
    uint noOfFragments;
};
struct NodeStruct {
    uint parentPtr;
    uint childPtr;
    uint childBit;
    uint modelBrickPtr;
    uint lightBit;
    uint lightBrickPtr;
    uint xPositive;
    uint xNegative;
    uint yPositive;
    uint yNegative;
    uint zPositive;
    uint zNegative;
};

layout(binding = 2) coherent buffer NodeBlock {
    NodeStruct node[];
};

uint getPtrOffset(ivec3 frameOffset) {
    return min(frameOffset.x, 1) * 1 
    + min(frameOffset.y, 1) * 2 + min(frameOffset.z, 1) * 4;
}

const int leafLevel = 3;
vec3 SearchOctree(vec3 pos, out uint nodeId) {
    nodeId = 0;
    vec3 prevSamplePos;
    vec3 samplePos = vec3(0.0f);
    vec3 refOffset;
    for(int i = 0; i < leafLevel; i++) {
        prevSamplePos = samplePos;
        samplePos = pos * levels[i];
        refOffset = samplePos - 2 * floor(prevSamplePos);
        uint child = node[nodeId].childPtr;
        uint childOffset = getPtrOffset(ivec3(refOffset));
        if(child == 0 || ((node[nodeId].childBit >> childOffset) & 1) == 0) {
            nodeId = INVALID;
            return refOffset;
        }
        nodeId = child + childOffset;
    }
    prevSamplePos = samplePos;
    samplePos = pos * levels[leafLevel];
    refOffset = samplePos - 2 * floor(prevSamplePos);
    uint leafOffset = getPtrOffset(ivec3(refOffset));
    uint childBit = node[nodeId].childBit;
    if(((childBit >> leafOffset) & 1) == 0) {
        nodeId = INVALID;
    }
    return refOffset;
}

bool isRayInCubeSpace(vec3 rayPosition) {
    return rayPosition.x >= 0.0f && rayPosition.x <=512.0f
        && rayPosition.y >= 0.0f && rayPosition.y <=512.0f
        && rayPosition.z >= 0.0f && rayPosition.z <=512.0f;
}

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

layout (location = 0) out vec4 FragColor;
void main() {    
    
    vec3 rOrigin = camPosition.xyz;
    float x = gl_FragCoord.x;
    float y = gl_FragCoord.y;
    vec3 viewPlaneCenter = camPosition.xyz + camForward.xyz * 0.5;
    vec3 U = normalize(cross(camForward.xyz, camUp.xyz));
    vec3 V = normalize(cross(U, camForward.xyz));
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
    vec3 refOffset;
    bool isRayInCube = true;
    //ray march
    do {
        
        if( x < 1 && y < 1) {
            logFragment(vec4(rayPosition, x), vec4(dir, y), leafIndex, 0, 0, isRayInCube ? 1 : 0);
        }
        rayPosition += dir;
        isRayInCube = isRayInCubeSpace(rayPosition);
        if(isRayInCube) {
            refOffset = SearchOctree(rayPosition, leafIndex);
        }
    } while(leafIndex == INVALID && isRayInCube);

    if(isRayInCube) {
        uint brickPtr = node[leafIndex].modelBrickPtr;
        ivec3 innerFramePos = ivec3(refOffset);
        ivec3 brickPos = innerFramePos + ivec3((brickPtr & 0x1FF) * 2, (brickPtr >> 9) * 2, 0);
        FragColor = imageLoad(colorBrick, brickPos); 
        
        if( x < 1 && y < 1) {
            logFragment(vec4(rayPosition, x), FragColor, leafIndex, uint(brickPos.x), uint(brickPos.y), isRayInCube ? 1 : 0);
        }

    } else {
        discard;
    }
}
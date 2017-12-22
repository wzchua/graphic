#version 450

#define M_PI 3.1415926535897932384626433832795

in vec3 wcPosition;   // Vertex position in scaled world space.

uniform int height;
uniform int width;
uniform vec3 camPosition;
uniform vec3 camForward;
uniform vec3 camUp;

layout(binding = 4, RGBA8) uniform image3D colorGrid;
layout(binding = 5, RGBA8) uniform image3D normalGrid;
layout(binding = 7, r32ui) uniform uimage3D gridCounter;

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
    
    vec3 rayPosition = rOrigin;
    bool hasHit = false;
    bool isRayInCube = true;
    //ray march
    do {
        rayPosition += dir;
        isRayInCube = isRayInCubeSpace(rayPosition);
        hasHit = imageLoad(gridCounter, ivec3(rayPosition)).r == 0;
    } while(hasHit == false && isRayInCube);

    if( x < 1 && y < 1) {
        logFragment(vec4(rayPosition, x), vec4(dir, y), leafIndex, uint(brickOffset.x), uint(brickOffset.y), isRayInCube ? 1 : 0);
    }
    if(isRayInCube) {
        FragColor = imageLoad(colorBrick, ivec3(rayPosition)); 
        
        if( x < 1 && y < 1) {
            logFragment(vec4(rayPosition, x), FragColor, leafIndex, uint(brickOffset.x), uint(brickOffset.y), isRayInCube ? 1 : 0);
        }

    } else {
        discard;
    }
}
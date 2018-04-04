#version 450

#define M_PI 3.1415926535897932384626433832795

in vec3 wcPosition;   // Vertex position in scaled world space.

layout(binding = 9) uniform RayCastBlock {
    mat4 ViewToVoxelMat;
    vec4 camPosition;
    vec4 camForward;
    vec4 camUp;
    int height;
    int width;
    int isEnergy;
    int gridDef;
    int mipLevel;
};

layout(binding = 7, std140) uniform LimitsUniformBlock {
    uint maxNoOfFragments;
    uint maxNoOfNodes;
    uint maxNoOfBricks;
    uint maxNoOfLogs;
};

layout(binding = 4, RGBA8) uniform image3D colorGrid;
//layout(binding = 5, RGBA8) uniform image3D normalGrid;

bool isRayInCubeSpace(vec3 rayPosition) {
    return rayPosition.x >= 0.0f && rayPosition.x <=512.0f
        && rayPosition.y >= 0.0f && rayPosition.y <=512.0f
        && rayPosition.z >= 0.0f && rayPosition.z <=512.0f;
}
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

layout(binding = 7) volatile buffer LogBlock {
    LogStruct logList[];
};

void logFragment(vec4 pos, vec4 color, uint nodeIndex, uint brickPtr, uint index1, uint index2) {
    uint index = atomicAdd(logCounter, 1);
    if(index < maxNoOfLogs) {
        LogStruct l;
        l.position = pos;
        l.color = color;
        l.nodeIndex = nodeIndex;
        l.brickPtr = brickPtr;
        l.index1 = index1;
        l.index2 = index2;
        logList[index] = l;
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
    
    //vec3 dir = normalize(wcPosition - (camPosition.xyz - camForward.xyz));
    vec3 rayPosition = rOrigin;
    bool hasHit = false;
    bool isRayInCube = true;
    vec4 color;
    if(gl_FragCoord.x < 1 && gl_FragCoord.y < 1) {        
        logFragment(vec4(rayPosition, 1.0f), vec4(gl_FragCoord.xyz, 1.0f), 0, 0, height, width);
    }
    //ray march
    do {
        rayPosition += dir;
        isRayInCube = isRayInCubeSpace(rayPosition);
        color = imageLoad(colorGrid, ivec3(rayPosition));
        hasHit = color.a != 0.0f;
        if(gl_FragCoord.x < 1 && gl_FragCoord.y < 1) {        
            logFragment(vec4(rayPosition, 1.0f), color, uint(hasHit), 0, height, width);
        }
    } while(!hasHit && isRayInCube);

    if(isRayInCube) {
        FragColor = color; 
        //FragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
        
    } else {
        discard;
    }
}
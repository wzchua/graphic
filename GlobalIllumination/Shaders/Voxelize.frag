#version 450
#extension GL_ARB_shader_atomic_counter_ops : require

in vec3 wcPosition;   // Vertex position in scaled world space.
in vec3 wcNormal;     // Vertex normal in world space.
in vec2 fTexCoord;

layout(binding = 0) uniform atomic_uint fragListPtr;
//initlized at 9
layout(binding = 1) uniform atomic_uint nodePtr;
// 0
layout(binding = 2) uniform atomic_uint brickPtrX;
layout(binding = 3) uniform atomic_uint brickPtrY;

struct FragmentStruct {
    vec4 position;
    vec4 color;
};

layout(binding = 0) buffer FragmentListBlock {
    FragmentStruct frag[];
};

struct NodeStruct {
    uint childPtr;
    uint brickPtrX;
    uint brickPtrY;
};

layout(binding = 1) volatile buffer NodeBlock {
    NodeStruct node[];
};
struct SyncStruct {
    uint treadInvoked;
};

layout(binding = 2) coherent buffer SyncBlock {
    SyncStruct sync[];
};


layout(r32ui) uniform uimage3D lock3D;

uniform bool useBumpMap;
layout (binding=1) uniform sampler2D diffuseTexture;
layout (binding=2) uniform sampler2D alphaTexture;
layout (binding=3) uniform sampler2D heightTexture;

uniform vec3 Diffuse;

layout(binding = 0, RGBA8) uniform image3D texture3D;

const vec2 size = vec2(2.0,0.0);
const ivec3 off = ivec3(-1,0,1);
const int length = 400;
const int width = 400;

bool isToDefer = false;

void deferFragment(vec4 color) {
    uint index = atomicCounterIncrement(fragListPtr);
    frag[index].position = vec4(wcPosition + vec3(256.0), 1.0f);
    frag[index].color = color;
    discard;
}

uint getChildPtr(uint parentPtr, ivec3 frameOffset) {
    //map to node space where voxel center is at corner
    ivec3 nodeOffset;
    nodeOffset.x = min(frameOffset.x, 1);
    nodeOffset.y = min(frameOffset.y, 1);
    nodeOffset.z = min(frameOffset.z, 1);
    uint ptrOffset = nodeOffset.x * 1 + nodeOffset.y * 2 + nodeOffset.z * 4;

    return ptrOffset + node[parentPtr].childPtr;
}

void generateNode(uint parentPtr, bool isLeaf) {
    if(node[parentPtr].childPtr != 0) {
        return;
    }
    uint isGenerating = atomicCompSwap(sync[parentPtr].treadInvoked, 0, 1);
    if(isGenerating == 1) {
        isToDefer = true;
        return;
    } else if(isGenerating == 2) {
        return;
    }
    if(!isLeaf) {
        atomicExchange(node[parentPtr].childPtr, atomicCounterAddARB(nodePtr, 8));
    }
    node[parentPtr].brickPtrX = atomicCounterIncrement(brickPtrX);
    atomicCompSwap(sync[parentPtr].treadInvoked, 1, 2);    
}

uniform float levels[9] = float[9](0.001953125f, 0.00390625f, 0.0078125f,
                                0.015625f, 0.03125f, 0.0625f,
                                0.125f, 0.25f, 0.5f);
                                
ivec3[9] computeLevelOffset() {
    ivec3 frameOffset = ivec3(0);
    ivec3 prevFrameOffset = ivec3(0);
    ivec3 levelOffsets[9];
    for(int i = 0; i < 9; i++) {
        prevFrameOffset = frameOffset * 2;
        frameOffset = ivec3(wcPosition * levels[i]);
        levelOffsets[i] = frameOffset - prevFrameOffset;
    }
    return levelOffsets;
} 

//Generates a voxel list from rasterization
void main() {    
    if(texture(alphaTexture, fTexCoord).r < 0.5f) {
        discard;
    }
    vec3 nwcNormal = normalize(wcNormal);
    if(useBumpMap) {            
        float h11 = texture(heightTexture, fTexCoord).r;
        float h01 = textureOffset(heightTexture, fTexCoord, off.xy).r;
        float h21 = textureOffset(heightTexture, fTexCoord, off.zy).r;
        float h10 = textureOffset(heightTexture, fTexCoord, off.yx).r;
        float h12 = textureOffset(heightTexture, fTexCoord, off.yz).r;

        vec3 va = normalize(vec3(size.xy,h21-h01));
        vec3 vb = normalize(vec3(size.yx,h12-h10));

        nwcNormal = normalize(nwcNormal + cross(va, vb));
    }
    vec4 color = vec4(Diffuse * texture(diffuseTexture, fTexCoord).rgb, 1.0f);
    deferFragment(color);
    return;
    
    /*
    uint nodeIndex = 0;
    ivec3 levelOffsets[9];
    levelOffsets = computeLevelOffset();
    //root node is pregenerated
    for(int i = 1; i < 9; i++) {        
        nodeIndex = getChildPtr(nodeIndex, levelOffsets[i]);
        generateNode(nodeIndex, i == 8);
        if(isToDefer) { 
            deferFragment(color);
            return; 
        }
    }
    */
}
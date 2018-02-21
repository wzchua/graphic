#version 450
#extension GL_ARB_shader_atomic_counter_ops : require
#extension GL_ARB_bindless_texture : require

in vec3 wcPosition;   // Vertex position in scaled world space.
in vec3 wcNormal;     // Vertex normal in world space.
in vec2 fTexCoord;

layout(binding = 1) uniform MatBlock {
    sampler2D texAmbient;
    sampler2D texDiffuse;
    sampler2D texAlpha;
    sampler2D texHeight;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    int useBumpMap;
    float shininess;
};
layout(binding = 7, std140) uniform LimitsUniformBlock {
    uint maxNoOfFragments;
    uint maxNoOfNodes;
    uint maxNoOfBricks;
    uint maxNoOfLogs;
};

struct FragmentStruct {
    vec4 position;
    vec4 color;
    vec4 normal;
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
struct LogStruct {
    vec4 position;
    vec4 color;
    uint nodeIndex;
    uint brickPtr;
    uint index1;
    uint index2;
};

layout(binding = 0) coherent buffer FragmentListBlock {
    FragmentStruct frag[];
};
layout(binding = 1) coherent buffer CounterBlock {
    uint fragmentCounter;
    uint nodeCounter;
    uint brickCounter;
    uint leafCounter;
    uint logCounter;
    uint noOfFragments;
};
layout(binding = 2) coherent buffer NodeBlock {
    NodeStruct node[];
};
layout(binding = 3) coherent buffer LeafListBlock {
    uint leafList[];
}
layout(binding = 7) coherent buffer LogBlock {
    LogStruct logList[];
};

layout(binding = 4, r32ui) uniform coherent volatile uimage3D colorBrick;
layout(binding = 5, r32ui) uniform coherent volatile uimage3D normalBrick;

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

const vec2 size = vec2(2.0,0.0);
const ivec3 off = ivec3(-1,0,1);
const uint ISINPROCESS = 0 - 1;
const uint LEAFHOST = 0 - 2;
const float levels[10] = float[10](0.001953125f, 0.00390625f, 0.0078125f,
                                0.015625f, 0.03125f, 0.0625f,
                                0.125f, 0.25f, 0.5f, 1.0f);

uint getPtrOffset(ivec3 frameOffset) {
    return min(frameOffset.x, 1) * 1 
    + min(frameOffset.y, 1) * 2 + min(frameOffset.z, 1) * 4;
}

uint checkRoot() {    
    return 0;
}

uint checkAndInitializeNode(int currentLevel, vec3 absPos, uint parentNodeIndex) {    
    ivec3 frameOffset = ivec3(absPos * levels[currentLevel]);
    ivec3 prevFrameOffset = 2 * ivec3(absPos * levels[currentLevel - 1]);
    uint offset = getPtrOffset(frameOffset - prevFrameOffset);
    
    atomicOr(node[parentNodeIndex].childBit, 1 << offset);//flag child bit

    uint childIndex = atomicCompSwap(node[parentNodeIndex].childPtr, 0, ISINPROCESS);
    if(childIndex == ISINPROCESS) { //node is being created
        return ISINPROCESS;
    }

    if(childIndex == 0) { //node is unintialized
        childIndex = atomicAdd(nodeCounter, 8);
        node[parentNodeIndex].modelBrickPtr = atomicAdd(brickCounter, 1);
        for(int i = 0; i < 8; i++) {
            node[childIndex + i].parentPtr = nodeIndex;
        }
        atomicCompSwap(node[parentNodeIndex].childPtr, ISINPROCESS, childIndex);
    }
    return childIndex + offset;
}

void deferToFragList(vec3 pos, vec4 color, vec3 normal) {
    uint index = atomicAdd(fragmentCounter, 1);
    //uint index = atomicCounterIncrement(fragListPtr);
    if(index < maxNoOfFragments) {
        FragmentStruct f;
        f.position = vec4(floor(pos), 1.0f);
        f.color = color;
        f.normal = vec4(normal, 1.0f);
        frag[index] = f;
        //logFragment(vec4(pos, 1.0f), color, 0, 0, 0, 0);
    } else {
        atomicAdd(fragmentCounter, uint(-1));
        //atomicCounterDecrement(fragListPtr);
    }
}

// leaf host node host 2x2 voxels
uint checkAndInitializeLeafHost(uint leafIndex) {
    uint leafState = atomicCompSwap(node[leafIndex].childPtr, 0, ISPROCESS);
    if(leafState == ISPROCESS){
        return ISPROCESS;
    }

    if(leafState == 0) {
        node[leafIndex].modelBrickPtr = atomicAdd(brickCounter, 1);
        uint index = atomicAdd(leafCounter, 1);
        leafList[index] = leafIndex;
        atomicCompSwap(node[leafIndex].childPtr, ISPROCESS, LEAFHOST);
    }
    
    return leafIndex;
}
//Referenced from: OpenGL Insight Chapter 22
vec4 convRGBA8ToVec4( uint val) {
    return vec4 ( float (( val &0x000000FF)) , float (( val &0x0000FF00) >>8U) , float (( val &0x00FF0000) >>16U) , float (( val &0xFF000000) >>24U) );
}
uint convVec4ToRGBA8( vec4 val) {
    return ( uint ( val.w) &0x000000FF) <<24U | ( uint( val.z) &0x000000FF) <<16U | ( uint( val.y ) &0x000000FF) <<8U | ( uint( val.x) &0x000000FF);
}
void imageAtomicRGBA8Avg( layout ( r32ui ) coherent volatile uimage3D imgUI , ivec3 coords , vec4 val ) {
    val.rgb *=255.0f; // Optimise following calculations
    uint newVal = convVec4ToRGBA8( val );
    uint prevStoredVal = 0; uint curStoredVal;
    // Loop as long as destination value gets changed by other threads
    while ( ( curStoredVal = imageAtomicCompSwap( imgUI , coords , prevStoredVal , newVal )) != prevStoredVal) {
        prevStoredVal = curStoredVal;
        vec4 rval = convRGBA8ToVec4( curStoredVal);
        rval.xyz =( rval.xyz * rval.w) ; // Denormalize
        vec4 curValF = rval + val; // Add new value
        curValF.xyz /=( curValF.w); // Renormalize
        newVal = convVec4ToRGBA8( curValF );
    }
}

// includes -127.0f to 127.0f
vec4 convXYZWToVec4( uint val) {
    vec3 xyz = vec3(float (( val &0x000000FF)) , float (( val &0x0000FF00) >>8U) , float (( val &0x00FF0000) >>16U));
    xyz = xyz - 128.0f;
    return vec4 (  xyz, float (( val &0xFF000000) >>24U) );
}
uint convVec4ToXYZW( vec4 val) {
    val.xyz = (val.xyz + 128.0f);
    return ( uint ( val.w) &0x000000FF) <<24U | ( uint( val.z) &0x000000FF) <<16U | ( uint( val.y ) &0x000000FF) <<8U | ( uint( val.x) &0x000000FF);
}
void imageAtomicXYZWAvg( layout ( r32ui ) coherent volatile uimage3D imgUI , ivec3 coords , vec4 val ) {
    val.rgb *= 127.0f;
    uint newVal = convVec4ToXYZW( val );
    uint prevStoredVal = 0; uint curStoredVal;
    // Loop as long as destination value gets changed by other threads
    while ( ( curStoredVal = imageAtomicCompSwap( imgUI , coords , prevStoredVal , newVal )) != prevStoredVal) {
        prevStoredVal = curStoredVal;
        vec4 rval = convXYZWToVec4( curStoredVal);
        rval.xyz =( rval.xyz * rval.w) ; // Denormalize
        vec4 curValF = rval + val; // Add new value
        curValF.xyz /=( curValF.w); // Renormalize
        newVal = convVec4ToXYZW( curValF );
    }
}

void addToOctree(vec3 pos, vec4 color, vec3 normal) {
    //pos from 0 to 512
    //check node 0: 0 to 1 do nothing
    //check node 1: 0 to 2
    //check node 2: 0 to 4
    //check node 3: 0 to 8
    //check node 4: 0 to 16
    //check node 5: 0 to 32
    //check node 6: 0 to 64
    //check node 7: 0 to 128
    //check node 8: 0 to 256
    //check node 9: 0 to 512
    uint parentNodeIndex = checkRoot();
    uint childNodeIndex = 0;
    for(int i = 1; i < 9; i++) {        
        //check node i: 0 to 2^i
        childNodeIndex = checkAndInitializeNode(i, pos, parentNodeIndex);
        if(childNodeIndex == ISINPROCESS) {
            deferToFragList(pos, color, normal);
            return;
        } else {
            //descend the octree
            parentNodeIndex = childNodeIndex;
        }
    }
    //add to leaf 
    uint leafState = checkAndInitializeLeafHost(childNodeIndex);
    if(leafState == ISINPROCESS) {
        deferToFragList(pos, color, normal);
        return;
    }
    uint brickPtr = node[childNodeIndex].modelBrickPtr;
    uint bx = (brickPtr & 0x1FF) * 2;
    uint by = (brickPtr >> 9) * 2;  

    ivec3 frameOffset = ivec3(pos * levels[9]);
    ivec3 prevFrameOffset = 2 * ivec3(pos * levels[8]);
    ivec3 innerFrameOffset = frameOffset - prevFrameOffset;
    ivec3 texturePos = ivec3(bx + innerFrameOffset.x, by + innerFrameOffset.y, innerFrameOffset.z);
    uint leafOffset = getPtrOffset(innerFrameOffset);
    atomicOr(node[childNodeIndex].childBit, 1 << leafOffset);
    imageAtomicRGBA8Avg(colorBrick, texturePos, color);
    imageAtomicXYZWAvg(normalBrick, texturePos, vec4(normal, 1.0f));
}

//Generates a voxel list from rasterization
void main() {    
    if(texture(texAlpha, fTexCoord).r < 0.5f) {
        discard;
    }
    vec3 nwcNormal = normalize(wcNormal);
    if(useBumpMap == 1) {            
        float h11 = texture(texHeight, fTexCoord).r;
        float h01 = textureOffset(texHeight, fTexCoord, off.xy).r;
        float h21 = textureOffset(texHeight, fTexCoord, off.zy).r;
        float h10 = textureOffset(texHeight, fTexCoord, off.yx).r;
        float h12 = textureOffset(texHeight, fTexCoord, off.yz).r;

        vec3 va = normalize(vec3(size.xy,h21-h01));
        vec3 vb = normalize(vec3(size.yx,h12-h10));

        nwcNormal = normalize(nwcNormal + cross(va, vb));
    }
    vec4 color = vec4(diffuse.rgb * texture(texDiffuse, fTexCoord).rgb, 1.0f);
    addToOctree(wcPosition, color, nwcNormal);
}
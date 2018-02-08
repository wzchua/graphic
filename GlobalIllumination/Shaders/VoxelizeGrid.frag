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
    uint maxNoOfLogs;
    uint maxNoOfFragments;
};

layout(binding = 4, r32ui) uniform coherent volatile uimage3D colorBrick;
layout(binding = 5, r32ui) uniform coherent volatile uimage3D normalBrick;
layout(binding = 7, r32ui) uniform coherent volatile uimage3D fragmentImageCounter;


layout(binding = 1) coherent buffer CounterBlock {
    uint fragmentCounter;
    uint logCounter;
};
layout(binding = 2) coherent buffer VoxelListBlock {
    vec4 voxelList[];
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

const vec2 size = vec2(2.0,0.0);
const ivec3 off = ivec3(-1,0,1);

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

void addToGrid(vec4 color, vec3 normal) {
    ivec3 pos = ivec3(wcPosition);    
    imageAtomicRGBA8Avg(colorBrick, pos, color);
    imageAtomicXYZWAvg(normalBrick, pos, vec4(normal, 1.0f));
    uint count = imageAtomicAdd(fragmentImageCounter, pos, 1);
    if(count == 0) {
        uint index = atomicAdd(fragmentCounter, 1);
        voxelList[index] = vec4(pos, 1.0f);
    }
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
    addToGrid(color, nwcNormal);
    //logFragment(vec4(convRG16ToVec2(convVec2ToRG16(nwcNormal.rg)), nwcNormal.rg), vec4(convRG16ToVec2(convVec2ToRG16(color.rg)), color.rg), 0, 0, 0, 0);
}
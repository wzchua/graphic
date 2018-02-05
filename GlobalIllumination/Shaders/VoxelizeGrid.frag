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
layout(binding = 0, r32ui) coherent uniform uimage3D rgColorBrick;
layout(binding = 1, r32ui) coherent uniform uimage3D baColorBrick;
layout(binding = 2, r32ui) coherent uniform uimage3D xyNormalBrick;
layout(binding = 3, r32ui) coherent uniform uimage3D zwNormalBrick;
layout(binding = 7, r32ui) uniform uimage3D fragmentImageCounter;

layout(binding = 1) coherent buffer CounterBlock {
    uint fragmentCounter;
    uint logCounter;
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

uint convVec2ToRG16( vec2 val) {
    val = val * 255.0;
return  ( uint( val .y ) &0x0000FFFF) <<16U | ( uint( val .x ) &0x0000FFFF);
}

vec2 convRG16ToVec2( uint val) {  
  return  vec2 ( float(val & 0x0000FFFF), float((val & 0xFFFF0000) >>16U)) /255.0;
}
void addToGrid(vec4 color, vec3 normal) {
    ivec3 pos = ivec3(wcPosition);
    imageAtomicAdd(rgColorBrick, pos, convVec2ToRG16(color.rg));
    imageAtomicAdd(baColorBrick, pos, convVec2ToRG16(color.ba));
    imageAtomicAdd(xyNormalBrick, pos, convVec2ToRG16(normal.xy));
    imageAtomicAdd(zwNormalBrick, pos, convVec2ToRG16(vec2(normal.z, 1.0f)));
    uint count = imageAtomicAdd(fragmentImageCounter, pos, 1);
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
    //logFragment(vec4(wcPosition, 1.0f), color, 0, 0, 0, 0);
}
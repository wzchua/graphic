#version 450
#extension GL_ARB_shader_atomic_counter_ops : require

in vec3 wcPosition;   // Vertex position in scaled world space.
in vec3 wcNormal;     // Vertex normal in world space.
in vec2 fTexCoord;
flat in int axis;

layout(binding = 0) uniform atomic_uint fragListPtr;
layout(binding = 7) uniform atomic_uint logPtr;

struct FragmentStruct {
    vec4 position;
    vec4 color;
    vec4 normal;
};

layout(binding = 0) buffer FragmentListBlock {
    FragmentStruct frag[];
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

void deferFragment(vec4 color, vec3 normal) {
    uint index = atomicCounterIncrement(fragListPtr);
    frag[index].position = vec4(floor(wcPosition), 1.0f);
    frag[index].color = color;
    frag[index].normal = vec4(normal, 1.0f);
    logFragment(vec4(wcPosition, 1.0f), color, 0, 0, 0, 0);
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
    deferFragment(color, nwcNormal);
}
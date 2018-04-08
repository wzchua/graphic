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

layout(binding = 3, std140) uniform LimitsUniformBlock {
    uint maxNoOfFragments;
    uint maxNoOfNodes;
    uint maxNoOfBricks;
    uint maxNoOfLogs;
};
layout(binding = 4, std140) uniform VoxelizeCascadedBlock{
    mat4 voxelToClipmapL0Mat;
    mat4 voxelToClipmapL1Mat;
    mat4 voxelToClipmapL2Mat;
    vec4 level0min;
    vec4 level0max;
    vec4 level1min;
    vec4 level1max;
    vec4 level2min;
    vec4 level2max;
};
layout(binding = 0) coherent buffer CounterBlock{
    uint fragmentCounter;
    uint nodeCounter;
    uint brickCounter;
    uint leafCounter;
    uint logCounter;
    uint noOfFragments;
};

layout(binding = 0) uniform sampler3D colorGridL0;
layout(binding = 1) uniform sampler3D normalGridL0;
layout(binding = 2) uniform sampler3D lightDirGridL0;
layout(binding = 3) uniform usampler3D lightEnergyGridL0;
layout(binding = 4) uniform sampler3D colorGridL1;
layout(binding = 5) uniform sampler3D normalGridL1;
layout(binding = 6) uniform sampler3D lightDirGridL1;
layout(binding = 7) uniform usampler3D lightEnergyGridL1;
layout(binding = 8) uniform sampler3D colorGridL2;
layout(binding = 9) uniform sampler3D normalGridL2;
layout(binding = 10) uniform sampler3D lightDirGridL2;
layout(binding = 11) uniform usampler3D lightEnergyGridL2;

struct LogStruct {
    vec4 position;
    vec4 color;
    uint nodeIndex;
    uint brickPtr;
    uint index1;
    uint index2;
};
layout(binding = 1,std430) coherent buffer LogBlock{
    uint maxLogCount; uint padding[3];
    LogStruct logList[];
};

void logFragment(vec4 pos, vec4 color, uint nodeIndex, uint brickPtr, uint index1, uint index2) {
    uint index = atomicAdd(logCounter, 1);
    if(index < maxLogCount) {        
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

bool isRayInCubeSpace(vec3 rayPosition, int level) {
    if(level == 2) {
        return rayPosition.x >= 0.0f && rayPosition.x <=511.0f
            && rayPosition.y >= 0.0f && rayPosition.y <=511.0f
            && rayPosition.z >= 0.0f && rayPosition.z <=511.0f;
    } else if(level == 1) {
        return rayPosition.x >= level1min.x  && rayPosition.x <=level1max.x
            && rayPosition.y >= level1min.y && rayPosition.y <=level1max.y
            && rayPosition.z >= level1min.z && rayPosition.z <=level1max.z;
    } else {
        return rayPosition.x >= level0min.x  && rayPosition.x <=level0max.x
            && rayPosition.y >= level0min.y && rayPosition.y <=level0max.y
            && rayPosition.z >= level0min.z && rayPosition.z <=level0max.z;
    }
}
//inputPos in world space
int findMinLevel(vec3 inputPos) {    
    int level;
    if(inputPos.x < level1min.x || inputPos.x > level1max.x
        || inputPos.y < level1min.y || inputPos.y > level1max.y
        || inputPos.z < level1min.z || inputPos.z > level1max.z) {        
        level = 2;
    } else if(inputPos.x < level0min.x || inputPos.x > level0max.x
        || inputPos.y < level0min.y || inputPos.y > level0max.y
        || inputPos.z < level0min.z || inputPos.z > level0max.z) {
        level = 1;
    } else {
        level = 0;
    }
    return level;
}
vec4 transformEnergyToColor(uint energy) {
    if(energy == 0) {
        return vec4(0.0f);
    } else if(energy <= 65535) {
        return vec4(0.0f, 0.0f, float(energy) / 65535.0f, 1.0f);
    } else if(energy <= 131071) {
        return vec4(0.0f, float(energy) / 131071.0f, 0.0f, 1.0f);
    } else if(energy <= 262143) {
        return vec4(float(energy) / 262143.0f, 0.0f, 0.0f, 1.0f);
    } else {
        return vec4(vec3(float(energy) / 536870911.0f), 1.0f);
    }
}
vec4 mapTo01(vec4 val) {
    val.xyz *= 0.5f + 0.5f;
    return val;
}
vec4 loadColor(vec3 pos, int level) {
    vec4 clipPos; vec4 val;
    if(level == 0) {
        clipPos = (voxelToClipmapL0Mat * vec4(pos, 1.0f));
        clipPos.xyz /= float(1 << mipLevel);
        if(isEnergy == 1) {
            return texelFetch(colorGridL0, ivec3(clipPos.xyz), mipLevel);
        } else if(isEnergy == 2) {            
            val = texelFetch(normalGridL0, ivec3(clipPos.xyz), mipLevel);
            val.xyz *= 0.5f + 0.5f;
            return val;
        } else if(isEnergy == 3) {
            val = texelFetch(lightDirGridL0, ivec3(clipPos.xyz), mipLevel);
            val.xyz *= 0.5f + 0.5f;
            return val;
        } else {            
            return transformEnergyToColor(texelFetch(lightEnergyGridL0, ivec3(clipPos.xyz), mipLevel).r);
        }
    } else if(level == 1) {
        clipPos = (voxelToClipmapL1Mat * vec4(pos, 1.0f));
        clipPos.xyz /= float(1 << mipLevel);
        if(isEnergy == 1) {
            return texelFetch(colorGridL1, ivec3(clipPos.xyz), mipLevel);
        } else if(isEnergy == 2) {            
            val =  texelFetch(normalGridL1, ivec3(clipPos.xyz), mipLevel);
            val.xyz *= 0.5f + 0.5f;
            return val;
        } else if(isEnergy == 3) {
            val =  texelFetch(lightDirGridL1, ivec3(clipPos.xyz), mipLevel);
            val.xyz *= 0.5f + 0.5f;
            return val;
        } else {            
            return transformEnergyToColor(texelFetch(lightEnergyGridL1, ivec3(clipPos.xyz), mipLevel).r);
        }
    } else {
        clipPos = (voxelToClipmapL2Mat * vec4(pos, 1.0f));
        clipPos.xyz /= float(1 << mipLevel);
        if(isEnergy == 1) {
            return texelFetch(colorGridL2, ivec3(clipPos.xyz), mipLevel);
        } else if(isEnergy == 2) {            
            val = texelFetch(normalGridL2, ivec3(clipPos.xyz), mipLevel);
            val.xyz *= 0.5f + 0.5f;
            return val;
        } else if(isEnergy == 3) {
            val = texelFetch(lightDirGridL2, ivec3(clipPos.xyz), mipLevel);
            val.xyz *= 0.5f + 0.5f;
            return val;
        } else {            
            return transformEnergyToColor(texelFetch(lightEnergyGridL2, ivec3(clipPos.xyz), mipLevel).r);
        }
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
    vec4 color; int level = gridDef;

    // if(gl_FragCoord.x < 1 && gl_FragCoord.y < 1) {        
    //     logFragment(vec4(rayPosition, 1.0f), level0min, 0, 0, height, width);
    //     logFragment(vec4(rayPosition, 1.0f), level0max, 0, 0, height, width);
    //     logFragment(vec4(rayPosition, 1.0f), level1min, 0, 0, height, width);
    //     logFragment(vec4(rayPosition, 1.0f), level1max, 0, 0, height, width);
    //     logFragment(vec4(rayPosition, 1.0f), level2min, 0, 0, height, width);
    //     logFragment(vec4(rayPosition, 1.0f), level2max, 0, 0, height, width);
    // }
    //ray march
    do {
        rayPosition += dir;
        if(!isRayInCubeSpace(rayPosition, level)) {
            break;
        }
        color = loadColor(rayPosition, level);
        hasHit = color.a != 0.0f;
        // if(gl_FragCoord.x < 1 && gl_FragCoord.y < 1) {    
        //     logFragment(vec4(rayPosition, 1.0f), color, uint(hasHit), isEnergy, height, width);    
        // }
    } while(!hasHit);

    if(isRayInCube) {
        FragColor = color; 
        
    } else {
        discard;
    }
}
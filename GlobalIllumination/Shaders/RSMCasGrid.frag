#version 450
#extension GL_ARB_bindless_texture : require

in vec3 wcPosition;   // Fragment's 3D position in world space.
in vec3 wcNormal;

layout(binding = 2) uniform LightBlock {                            
    vec4 LightPosition; // Given in world space. Can be directional.
    vec4 LightAmbient; 
    vec4 LightDiffuse;
    vec4 LightSpecular;
    uint rad;
};
layout(binding = 3, std140) uniform VoxelizeMatrixBlock {
    mat4 WorldToVoxelMat;
    mat4 ViewProjMatrixXY; 
    mat4 worldToVoxelClipmapL0Mat;
    mat4 worldToVoxelClipmapL1Mat;
    mat4 worldToVoxelClipmapL2Mat;
};
layout(binding = 4, std140) uniform BoundariesBlock {
    vec4 level0min;
    vec4 level0max;
    vec4 level1min;
    vec4 level1max;
    vec4 level2min;
    vec4 level2max;
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

layout(binding = 0, r32ui) uniform coherent volatile uimage3D lightDirGridL0;
layout(binding = 1, r32ui) uniform coherent volatile uimage3D lightEnergyGridL0;
layout(binding = 2, r32ui) uniform coherent volatile uimage3D lightDirGridL1;
layout(binding = 3, r32ui) uniform coherent volatile uimage3D lightEnergyGridL1;
layout(binding = 4, r32ui) uniform coherent volatile uimage3D lightDirGridL2;
layout(binding = 5, r32ui) uniform coherent volatile uimage3D lightEnergyGridL2;

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


void main()
{
    vec3 pos;
    uimage3D lightEnergyGrid;
    uimage3D lightDirectionGrid;
    if(wcPosition.x < level1min.x || wcPosition.x > level1max.x
        || wcPosition.y < level1min.y || wcPosition.y > level1max.y
        || wcPosition.z < level1min.z || wcPosition.z > level1max.z) {

        pos = (worldToVoxelClipmapL2Mat * vec4(wcPosition, 1.0f)).xyz;
        lightEnergyGrid = lightEnergyGridL2;
        lightDirectionGrid = lightDirGridL2;
    } else if(wcPosition.x < level0min.x || wcPosition.x > level0max.x
        || wcPosition.y < level0min.y || wcPosition.y > level0max.y
        || wcPosition.z < level0min.z || wcPosition.z > level0max.z) {

        pos = (worldToVoxelClipmapL1Mat * vec4(wcPosition, 1.0f)).xyz;
        lightEnergyGrid = lightEnergyGridL1;
        lightDirectionGrid = lightDirGridL1;

    } else {        
        pos = (worldToVoxelClipmapL0Mat * vec4(wcPosition, 1.0f)).xyz;
        lightEnergyGrid = lightEnergyGridL0;
        lightDirectionGrid = lightDirGridL0;
    }

    vec3 lightDisplacement = wcPosition - LightPosition.xyz;
    vec3 lightDir = normalize(lightDisplacement);
    float dist = length(lightDisplacement);
    float distSq = dist * dist;
    //light energy & direction
    uint recievedEnergy = min( uint(float(rad) / (distSq * dot(lightDir, normalize(wcNormal)))), 1);
    imageAtomicAdd(lightEnergyGrid, pos, recievedEnergy);
    imageAtomicXYZWAvg(lightDirectionGrid, pos, vec4(lightDir, 1.0f));

}
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
    mat4 ViewProjMatrixZY; 
    mat4 ViewProjMatrixXZ;
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

layout(binding = 4, r32ui) uniform coherent volatile uimage3D lightDirBrick;
layout(binding = 5, r32ui) uniform coherent volatile uimage3D lightEnergyBrick;

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

void main()
{
    ivec3 pos = WorldToVoxelMat * wcPosition;

    uint parentNodeIndex = checkRoot();
    uint childNodeIndex = 0;
    ivec3 frameOffset;
    ivec3 prevFrameOffset;
    uint offset;
    for(int i = 1; i < 9; i++) {        
        //check node i: 0 to 2^i  
        frameOffset = ivec3(absPos * levels[i]);
        prevFrameOffset = 2 * ivec3(absPos * levels[i - 1]);
        offset = getPtrOffset(frameOffset - prevFrameOffset);    
        childNodeIndex = node[parentNodeIndex].childPtr;
        if(childNodeIndex == 0) {
            //error
            return;
        } else {
            //descend the octree
            parentNodeIndex = childNodeIndex + offset;
        }
    }
    //leaf    
    uint brickPtr = node[parentNodeIndex].modelBrickPtr;
    uint bx = (brickPtr & 0x1FF) * 2;
    uint by = (brickPtr >> 9) * 2;  
    
    frameOffset = ivec3(pos * levels[9]);
    prevFrameOffset = 2 * ivec3(pos * levels[8]);
    innerFrameOffset = frameOffset - prevFrameOffset;
    ivec3 texturePos = ivec3(bx + innerFrameOffset.x, by + innerFrameOffset.y, innerFrameOffset.z);
    uint leafOffset = getPtrOffset(innerFrameOffset);    
    atomicOr(node[childNodeIndex].lightBit, 1 << leafOffset);
    vec3 lightDisplacement = wcPosition - LightPosition.xyz;
    vec3 lightDir = normalize(lightDisplacement);
    float dist = length(lightDisplacement);
    float distSq = dist * dist;
    //light energy & direction
    uint recievedEnergy = min( uint(float(rad) / (distSq * dot(lightDir, normalize(wcNormal)))), 1);
    imageAtomicAdd(lightEnergyBrick, texturePos, recievedEnergy);
    imageAtomicXYZWAvg(lightDirBrick, texturePos, vec4(lightDir, 1.0f));
}
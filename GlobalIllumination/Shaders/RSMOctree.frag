#version 450
#extension GL_ARB_bindless_texture : require

in vec3 wcPosition;   // Fragment's 3D position in world space.
in vec3 ecNormal;
in vec2 fTexCoord;

layout(binding = 0, std140) uniform MatrixBlock {
    mat4 ModelViewMatrix;     // ModelView matrix.
    mat4 ModelViewProjMatrix; // ModelView matrix * Projection matrix.
    mat3 NormalMatrix;        // For transforming object-space direction 
};                                    //   vector to eye space.

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
const float levels[9] = float[9](0.00390625f, 0.0078125f,
                                0.015625f, 0.03125f, 0.0625f,
                                0.125f, 0.25f, 0.5f, 1.0f);

uint getPtrOffset(ivec3 frameOffset) {
    return min(frameOffset.x, 1) * 1 
    + min(frameOffset.y, 1) * 2 + min(frameOffset.z, 1) * 4;
}

uint checkRoot() {    
    return 0;
}
const int leafLevel = 8;
const vec2 size = vec2(2.0,0.0);
const ivec3 off = ivec3(-1,0,1);
void main()
{
    vec3 nwcNormal = wcNormal;
    if(useBumpMap == 1) {            
        float h11 = texture(texHeight, fTexCoord).r;
        float h01 = textureOffset(texHeight, fTexCoord, off.xy).r;
        float h21 = textureOffset(texHeight, fTexCoord, off.zy).r;
        float h10 = textureOffset(texHeight, fTexCoord, off.yx).r;
        float h12 = textureOffset(texHeight, fTexCoord, off.yz).r;

        vec3 va = normalize(vec3(size.xy,h21-h01));
        vec3 vb = normalize(vec3(size.yx,h12-h10));

        nwcNormal = nwcNormal + cross(va, vb);
    }
    nwcNormal = normalize(nwcNormal);

    vec3 pos = (WorldToVoxelMat * vec4(wcPosition, 1.0f)).xyz;
    uint nodeId = 0;
    vec3 prevSamplePos;
    vec3 samplePos = vec3(0.0f);
    vec3 refOffset;
    for(int i = 0; i < 8; i++) {
        prevSamplePos = samplePos;
        samplePos = pos * levels[i];
        refOffset = samplePos - 2 * floor(prevSamplePos);
        nodeId = node[nodeId].childPtr + getPtrOffset(ivec3(refOffset));
    }
    prevSamplePos = samplePos;
    samplePos = pos * levels[leafLevel];
    refOffset = samplePos - 2 * floor(prevSamplePos);
    uint brickPtr = node[nodeId].modelBrickPtr;
    ivec3 innerFramePos = ivec3(refOffset);
    ivec3 brickPos = innerFramePos + ivec3((brickPtr & 0x1FF) * 2, (brickPtr >> 9) * 2, 0);
    atomicOr(node[childNodeIndex].lightBit, 1 << getPtrOffset(innerFramePos));

    vec3 lightDisplacement = wcPosition - LightPosition.xyz;
    vec3 lightDir = normalize(lightDisplacement);
    float dist = length(lightDisplacement);
    float distSq = dist * dist;
    //light energy & direction
    uint recievedEnergy = min( uint(float(rad) / (distSq * dot(lightDir, normalize(nwcNormal)))), 1);
    imageAtomicAdd(lightEnergyBrick, brickPos, recievedEnergy);
    imageAtomicXYZWAvg(lightDirBrick, brickPos, vec4(lightDir, 1.0f));
}
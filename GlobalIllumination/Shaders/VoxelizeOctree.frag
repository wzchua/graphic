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

const vec2 size = vec2(2.0,0.0);
const ivec3 off = ivec3(-1,0,1);
const uint ISINPROCESS = 0 - 1;
const uint LEAFHOST = 0 - 2;
const float levels[10] = float[10](0.001953125f, 0.00390625f, 0.0078125f,
                                0.015625f, 0.03125f, 0.0625f,
                                0.125f, 0.25f, 0.5f, 1.0f);
//z=0  2  3   z=1  6  7
//     0  1        4  5
uint getPtrOffset(ivec3 frameOffset) {
    return min(frameOffset.x, 1) * 1 
    + min(frameOffset.y, 1) * 2 + min(frameOffset.z, 1) * 4;
}

uint checkRoot() {    
    return 0;
}

uint checkAndInitializeNode(uint parentNodeIndex) { 
    uint childIndex = atomicCompSwap(node[parentNodeIndex].childPtr, 0, ISINPROCESS);
    if(childIndex == ISINPROCESS) { //node is being created
        return ISINPROCESS;
    }

    if(childIndex == 0) { //node is unintialized
        childIndex = atomicAdd(nodeCounter, 8);
        if(childIndex + 8 >= maxNoOfNodes) {
            logFragment(vec4(55.0f), vec4(55.0f), 55, 55, 55, 55);
            discard;
        } else {                
            node[parentNodeIndex].valueIndex = atomicAdd(brickCounter, 1);
            for(int i = 0; i < 8; i++) {
                node[childIndex + i].parentPtr = parentNodeIndex;
            }
            // setup inner pointers
            atomicCompSwap(node[parentNodeIndex].childPtr, ISINPROCESS, childIndex);
        }
    }
    return childIndex;
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
    uint leafState = atomicCompSwap(node[leafIndex].childPtr, 0, ISINPROCESS);
    if(leafState == ISINPROCESS){
        return ISINPROCESS;
    }

    if(leafState == 0) {
        node[leafIndex].valueIndex = atomicAdd(brickCounter, 1);
        uint index = atomicAdd(leafCounter, 1);
        if(index < maxNoOfFragments) {
            leafList[index] = leafIndex;
        } else {
            atomicAdd(leafCounter, uint(-1));
        }
        atomicCompSwap(node[leafIndex].childPtr, ISINPROCESS, LEAFHOST);
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
void atomicColorAvg(uint index , vec4 val ) {
    val.rgb *=255.0f; // Optimise following calculations
    uint newVal = convVec4ToRGBA8( val );
    uint prevStoredVal = 0; uint curStoredVal;
    // Loop as long as destination value gets changed by other threads
    while ( ( curStoredVal = atomicCompSwap( nodeBrick[index].color , prevStoredVal , newVal )) != prevStoredVal) {
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
void atomicNormalAvg( uint index , vec4 val) {
    val.rgb *= 127.0f;
    uint newVal = convVec4ToXYZW( val );
    uint prevStoredVal = 0; uint curStoredVal;
    // Loop as long as destination value gets changed by other threads
    while ( ( curStoredVal = atomicCompSwap( nodeBrick[index].normal , prevStoredVal , newVal )) != prevStoredVal) {
        prevStoredVal = curStoredVal;
        vec4 rval = convXYZWToVec4( curStoredVal);
        rval.xyz =( rval.xyz * rval.w) ; // Denormalize
        vec4 curValF = rval + val; // Add new value
        curValF.xyz /=( curValF.w); // Renormalize
        newVal = convVec4ToXYZW( curValF );
    }
}
void atomicLightDirAvg( uint index , vec4 val) {
    val.rgb *= 127.0f;
    uint newVal = convVec4ToXYZW( val );
    uint prevStoredVal = 0; uint curStoredVal;
    // Loop as long as destination value gets changed by other threads
    while ( ( curStoredVal = atomicCompSwap( nodeBrick[index].lightDirection , prevStoredVal , newVal )) != prevStoredVal) {
        prevStoredVal = curStoredVal;
        vec4 rval = convXYZWToVec4( curStoredVal);
        rval.xyz =( rval.xyz * rval.w) ; // Denormalize
        vec4 curValF = rval + val; // Add new value
        curValF.xyz /=( curValF.w); // Renormalize
        newVal = convVec4ToXYZW( curValF );
    }
}
const int leafLevel = 9;
void addToOctree(vec3 pos, vec4 color, vec3 normal) {
    //pos from 0 to 512
    // Level 0 : 0.0 to 1.0
    // Level 1 : 0.0 to 2.0
    // Level 2 : 0.0 to 4.0
    // Level 3 : 0.0 to 8.0
    // Level 4 : 0.0 to 16.0
    // Level 5 : 0.0 to 32.0
    // Level 6 : 0.0 to 64.0
    // Level 7 : 0.0 to 128.0
    // Level 8 : 0.0 to 256.0
    // Level 9 : 0.0 to 512.0
    uint parentNodeIndex = checkRoot();
    uint childNodeIndex = 0;
    vec3 prevSamplePos;
    vec3 samplePos;
    vec3 refOffset;
    for(int i = 0; i < leafLevel; i++) {        
        prevSamplePos = samplePos;
        samplePos = pos * levels[i];
        refOffset = samplePos - 2 * floor(prevSamplePos);
        //check node i: 0 to 2^i
        childNodeIndex = checkAndInitializeNode(parentNodeIndex);
        if(childNodeIndex == ISINPROCESS) {
            deferToFragList(pos, color, normal);
            return;
        } else {
            //descend the octree
            uint offset =  getPtrOffset(ivec3(refOffset));
            atomicOr(node[parentNodeIndex].childBit, 1 << offset);//flag child bit
            parentNodeIndex = childNodeIndex + offset;
        }
    }
    //add to leaf 
    uint leafState = checkAndInitializeLeafHost(parentNodeIndex);
    if(leafState == ISINPROCESS) {
        deferToFragList(pos, color, normal);
        return;
    }
    prevSamplePos = samplePos;
    samplePos = pos * levels[leafLevel];
    refOffset = samplePos - 2 * floor(prevSamplePos);    
    ivec3 innerFramePos = ivec3(refOffset);
    atomicOr(node[parentNodeIndex].childBit, 1 << getPtrOffset(innerFramePos));

    uint valueIndex = node[parentNodeIndex].valueIndex;
    atomicColorAvg(valueIndex, color);
    atomicNormalAvg(valueIndex, vec4(normal, 1.0f));
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
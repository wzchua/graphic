#version 450
#extension GL_ARB_bindless_texture : require

in vec3 wcPosition;   // Fragment's 3D position in world space.
in vec3 wcNormal;

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

layout(binding = 4, rgba8) uniform coherent volatile image3D colorBrick;
layout(binding = 5, r32ui) uniform coherent volatile uimage3D normalBrick;
layout(binding = 6, r32ui) uniform coherent volatile uimage3D lightDirBrick;
layout(binding = 7, r32ui) uniform coherent volatile uimage3D lightEnergyBrick;

struct GaussianLobe {
    vec3 amplitude;
    vec3 axis;
    float sharpness;
}
//https://mynameismjp.wordpress.com/2016/10/09/sg-series-part-2-spherical-gaussians-101/
//http://www.jp.square-enix.com/tech/library/pdf/Fast%20Indirect%20illumination%20Using%20Two%20Virtual%20Spherical%20Gaussian%20Lights%20(Supplemental%20Material).pdf
GaussianLobe generateSG(vec3 amplitude, vec3 mean) {
    GaussianLobe g;
    g.amplitude = amplitude;
    g.axis = normalize(mean);
    float len = length(mean);
    g.sharpness = len / (1.0f - len); //Toksvig's filtering
    return g;
}

GaussianLobe Product(GaussianLobe g1, GaussianLobe g2) {
    GaussianLobe g;
    float l = g1.sharpness + g2.sharpness;
    vec3 u = (g1.sharpness * g1.axis + g2.sharpness * g2.axis) / (l);
    float uLength = length(u);

    g.axis = u * (1.0f / uLength);
    g.sharpness = l * uLength;
    g.amplitude = g1.amplitude * g2.amplitude * exp(l * (uLength - 1.0f));
    return g;
}

vec3 InnerProject(GaussianLobe g1, GaussianLobe g2) {
    float uLength = length(g1.sharpness * g1.axis + g2.sharpness * g2.axis);
    float eFactorized = exp(uLength - g1.sharpness - g2.sharpness);
    float factorized2 = 1.0f - exp(-2.0f * uLength);
    return (2.0f * Pi * g1.amplitude * g2.amplitude * eFactorized * factorized2) / uLength;
}

const float levels[10] = float[10](0.001953125f, 0.00390625f, 0.0078125f,
                                0.015625f, 0.03125f, 0.0625f,
                                0.125f, 0.25f, 0.5f, 1.0f);
uint getPtrOffset(ivec3 frameOffset) {
    return frameOffset.x * 1 + frameOffset.y * 2 + frameOffset.z * 4;
}
// 2.1 
uint findNode(vec3 pos, float size) {
    uint nodeId = 0;
    for(int i = 1; i < 9; i++) {
        if((1 << (10 - i)) < size) { //512, 256, 128, 64, 32, 16, 8, 4, 2, 1
            break;
        }
        //descendNode        
        ivec3 frameOffset = ivec3(pos * levels[i]);
        ivec3 prevFrameOffset = 2 * ivec3(pos * levels[i - 1]);
        uint offset = getPtrOffset(frameOffset - prevFrameOffset);
        nodeId = node[nodeId].childPtr + offset;
    }
    return nodeId;
}


vec4 ConeTrace(vec3 origin, vec3 dir, float coneSize) {
    float alpha = 0.0f;
    vec3 samplePos = origin + dir;
    while(alpha < 1.0f) {
        uint nodeId = findNode(samplePos, length(samplePos - origin) * coneSize);
        
    }
}

void main() 
{    
    ivec3 pos = (WorldToVoxelMat * vec4(wcPosition, 1.0f)).xyz;
    uint energy = 0;
    vec4 color;
}
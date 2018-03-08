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

layout(binding = 4) uniform sampler3D colorBrick;
layout(binding = 5) uniform sampler3D normalBrick;
layout(binding = 6) uniform sampler3D lightDirBrick;
layout(binding = 7) uniform usampler3D lightEnergyBrick;

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

const float levels[9] = float[9](0.00390625f, 0.0078125f,
                                0.015625f, 0.03125f, 0.0625f,
                                0.125f, 0.25f, 0.5f, 1.0f);
uint getPtrOffset(ivec3 frameOffset) {
    return frameOffset.x * 1 + frameOffset.y * 2 + frameOffset.z * 4;
}

const int leafLevel = 3;
vec3 SearchOctree(vec3 pos, float size, out uint nodeId) {
    nodeId = 0;
    vec3 prevSamplePos;
    vec3 samplePos = vec3(0.0f);
    vec3 refOffset;
    int reqLevel = 0;
    for(int i = 0; i <= leafLevel; i++) { // 512 * level[8] = 1, 256 * level[7] = 1
        if(size * levels[leafLevel - i] <= 1.0f) {
            reqLevel = i;
            break;
        }
    }
    for(int i = 0; i < reqLevel; i++) {
        prevSamplePos = samplePos;
        samplePos = pos * levels[i];
        refOffset = samplePos - 2 * floor(prevSamplePos);
        uint child = node[nodeId].childPtr;
        uint childOffset = getPtrOffset(ivec3(refOffset));
        if(child == 0 || ((node[nodeId].childBit >> childOffset) & 1) == 0) {
            nodeId = INVALID;
            return refOffset;
        }
        nodeId = child + childOffset;
    }
    prevSamplePos = samplePos;
    samplePos = pos * levels[reqLevel];
    refOffset = samplePos - 2 * floor(prevSamplePos);
    uint leafOffset = getPtrOffset(ivec3(refOffset));
    uint childBit = node[nodeId].childBit;
    if(((childBit >> leafOffset) & 1) == 0) {
        nodeId = INVALID;
    }
    return refOffset;
}

vec4 diffuseConeTrace(vec3 origin, vec3 dir, float coneSize) {
    float alpha = 0.0f;
    vec3 samplePos = origin + dir;
    uint nodeId = 0;
    vec3 color = vec3(0.0f);
    float cosPhi = 1.2;
    while(alpha < 1.0f) {
        vec3 refOffset = SearchOctree(samplePos, length(samplePos - origin) * coneSize, nodeId);    
        uint brickPtr = node[nodeId].modelBrickPtr;
        vec3 brickPos = refOffset + vec3((brickPtr & 0x1FF) * 2, (brickPtr >> 9) * 2, 0);
        vec4 c = texture(colorBrick, brickPos);
        vec4 n = 2 * texture(normalBrick, brickPos) - 1.0f;
        uint lEnergy = texture(lightEnergyBrick, brickPos).a;
        vec4 l = 2 * texture(lightDirBrick, brickPos) - 1.0f;

        GaussianLobe normalLobe = generateSG(vec3(1.0f), n.xyz);
        GaussianLobe lightLobe = generateSG(vec3(lEnergy), -l.xyz);
        GaussianLobe viewLobe;
        viewLobe.amplitude = vec3(1.0f);
        viewLobe.axis = dir;
        viewLobe.sharpness = 1/cosPhi^2;
        vec3 brdf = c.rgb / PI;
        vec3 convLightNormalView = InnerProject(viewLobe, Product(normalLobe, lightLobe));
        color += brdf * convLightNormalView;
        alpha = alpha + (1.0f - alpha) * c.a;
    }
}
vec4 specularConeTrace(vec3 origin, vec3 dir, float coneSize) {
    float alpha = 0.0f;
    vec3 samplePos = origin + dir;
    uint nodeId = 0;
    vec3 color = vec3(0.0f);
    float cosPhi = 1.2;
    while(alpha < 1.0f) {
        vec3 refOffset = SearchOctree(samplePos, length(samplePos - origin) * coneSize, nodeId);    
        uint brickPtr = node[nodeId].modelBrickPtr;
        vec3 brickPos = refOffset + vec3((brickPtr & 0x1FF) * 2, (brickPtr >> 9) * 2, 0);
        vec4 c = texture(colorBrick, brickPos);
        vec4 n = 2 * texture(normalBrick, brickPos) - 1.0f;
        uint lEnergy = texture(lightEnergyBrick, brickPos).a;
        vec4 l = 2 * texture(lightDirBrick, brickPos) - 1.0f;

        GaussianLobe normalLobe = generateSG(vec3(1.0f), n.xyz);
        GaussianLobe lightLobe = generateSG(vec3(lEnergy), -l.xyz);
        GaussianLobe viewLobe;
        viewLobe.amplitude = vec3(1.0f);
        viewLobe.axis = dir;
        viewLobe.sharpness = 1/cosPhi^2;
        vec3 brdf = c.rgb / PI;
        vec3 convLightNormalView = InnerProject(viewLobe, Product(normalLobe, lightLobe));
        color += brdf * convLightNormalView;
        alpha = alpha + (1.0f - alpha) * c.a;
    }
}
layout (location = 0) out vec4 FragColor;

void main() 
{    
    vec3 pos = (WorldToVoxelMat * vec4(wcPosition, 1.0f)).xyz;
    uint energy = 0;
    // 4x 60 from normal + 1 at normal;
    vec3 orthoX = findOrthoVector(wcNormal);
    vec3 orthoY = cross(wcNormal, orthoX); 
    vec3 diffuseColor += diffuseConeTrace(pos, wcNormal, 1.0f);
    diffuseColor += diffuseConeTrace(pos, mix(wcNormal, orthoX, 0.3), 1.0f);
    diffuseColor += diffuseConeTrace(pos, mix(wcNormal, -orthoX, 0.3), 1.0f);
    diffuseColor += diffuseConeTrace(pos, mix(wcNormal, orthoY, 0.3), 1.0f);
    diffuseColor += diffuseConeTrace(pos, mix(wcNormal, -orthoY, 0.3), 1.0f);
    vec3 view  = normalize(wcPosition - eyePos);
    vec3 specularColor = specularConeTrace(pos, reflect(view, wcNormal), 0.5f);

    FragColor = vec4(diffuseColor + specularColor, 1.0f);
}
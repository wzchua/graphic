layout(binding = 0) uniform sampler3D colorBrickL0;
layout(binding = 1) uniform sampler3D normalBrickL0;
layout(binding = 2) uniform sampler3D lightDirBrickL0;
layout(binding = 3) uniform usampler3D lightEnergyBrickL0;
layout(binding = 4) uniform sampler3D colorBrickL1;
layout(binding = 5) uniform sampler3D normalBrickL1;
layout(binding = 6) uniform sampler3D lightDirBrickL1;
layout(binding = 7) uniform usampler3D lightEnergyBrickL1;
layout(binding = 8) uniform sampler3D colorBrickL2;
layout(binding = 9) uniform sampler3D normalBrickL2;
layout(binding = 10) uniform sampler3D lightDirBrickL2;
layout(binding = 11) uniform usampler3D lightEnergyBrickL2;

layout(binding = 3) uniform CameraBlock {
    vec4 camPosition;
    vec4 camForward;
    vec4 camUp;
    int height;
    int width;
};
layout (location = 0) out vec4 FragColor;

#define PI           3.14159265358979323846

struct GaussianLobe {
    vec3 amplitude;
    vec3 axis;
    float sharpness;
};
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
    return (2.0f * PI * g1.amplitude * g2.amplitude * eFactorized * factorized2) / uLength;
}
//inputPos in world space
vec3 findPosAndLevel(vec3 inputPos, int size, out int level) {    
    vec3 pos;
    mat4 voxelToReqClipmap;
    if(size == 1) {
        voxelToReqClipmap = voxelToClipmapL0Mat;
    } else if (size == 2) {
        voxelToReqClipmap = voxelToClipmapL1Mat;
    } else {
        voxelToReqClipmap = voxelToClipmapL2Mat;
    }

    if(inputPos.x < level2min.x || inputPos.x > level2max.x
        || inputPos.y < level2min.y || inputPos.y > level2max.y
        || inputPos.z < level2min.z || inputPos.z > level2max.z) {
            //error
            pos = vec3(-1.0);
            level = -1;
    } else if(inputPos.x < level1min.x || inputPos.x > level1max.x
        || inputPos.y < level1min.y || inputPos.y > level1max.y
        || inputPos.z < level1min.z || inputPos.z > level1max.z) {

        pos = (voxelToClipmapL2Mat * WorldToVoxelMat * vec4(inputPos, 1.0f)).xyz;
        level = 2;
    } else if(inputPos.x < level0min.x || inputPos.x > level0max.x
        || inputPos.y < level0min.y || inputPos.y > level0max.y
        || inputPos.z < level0min.z || inputPos.z > level0max.z) {

        pos = (voxelToClipmapL1Mat * WorldToVoxelMat * vec4(inputPos, 1.0f)).xyz;
        level = 1;
    } else {
        pos = (voxelToClipmapL0Mat * WorldToVoxelMat * vec4(inputPos, 1.0f)).xyz;
        level = 0;
    }
    return pos;
}

// origin & dir in world space
vec3 diffuseConeTrace(vec3 origin, vec3 dir, float coneSize) {
    float alpha = 0.0f;
    int level = 0;
    vec3 clipOrigin = findPosAndLevel(origin, level);
    if(level < 0) {
        return vec4(0.0f);
    }
    vec3 adjustedDir = pow(2, level) * dir; // lengthen dir when traversing through larger grid dim
    vec3 rayWorldPos = origin + adjustedDir;
    vec3 color = vec3(0.0f);
    float cosPhi = 1.2;
    sampler3D colorClip = colorBrickL0;
    sampler3D normalClip = normalBrickL0;
    sampler3D lightDirClip = lightDirBrickL0;
    usampler3D lightEnergyClip = lightEnergyBrickL0;
    while(alpha < 1.0f) {
        clipOrigin = findPosAndLevel(rayWorldPos, level);
        if(level < 0) {
            return vec4(0.0f);
        } else if(level == 0) {
            colorClip = colorBrickL0;
            normalClip = normalBrickL0;
            lightDirClip = lightDirBrickL0;
            lightEnergyClip = lightEnergyBrickL0;
        } else if(level == 1) {
            colorClip = colorBrickL1;
            normalClip = normalBrickL1;
            lightDirClip = lightDirBrickL1;
            lightEnergyClip = lightEnergyBrickL1;
        } else {
            colorClip = colorBrickL2;
            normalClip = normalBrickL2;
            lightDirClip = lightDirBrickL2;
            lightEnergyClip = lightEnergyBrickL2;
        }
        vec4 c = texture(colorClip, clipOrigin);
        vec4 n = 2 * texture(normalClip, clipOrigin) - 1.0f;
        uint lEnergy = texture(lightEnergyClip, clipOrigin).a;
        vec4 l = 2 * texture(lightDirClip, clipOrigin) - 1.0f;

        GaussianLobe normalLobe = generateSG(vec3(1.0f), n.xyz);
        GaussianLobe lightLobe = generateSG(vec3(lEnergy), -l.xyz);
        GaussianLobe viewLobe;
        viewLobe.amplitude = vec3(1.0f);
        viewLobe.axis = dir;
        viewLobe.sharpness = 1/(cosPhi * cosPhi);
        vec3 brdf = c.rgb / PI;
        vec3 convLightNormalView = InnerProject(viewLobe, Product(normalLobe, lightLobe));

        color += brdf * convLightNormalView;
        alpha = alpha + (1.0f - alpha) * c.a;

        adjustedDir = pow(2, level) * dir;
        rayWorldPos += adjustedDir;
    }
    return color;
}

vec3 findOrthoVector(vec3 v) {
    vec3 u = cross(v, vec3(1, 0, 0));
    if(dot(u, v) == 1.0f) {
        u = cross(v, vec3(0, 1, 0));
    }
    return u;
}

void main() 
{    
    vec3 pos = (WorldToVoxelMat * vec4(wcPosition, 1.0f)).xyz;
    uint energy = 0;
    // 4x 60 from normal + 1 at normal;
    wcNormal = normalize(wcNormal);
    vec3 orthoX = findOrthoVector(wcNormal);
    vec3 orthoY = cross(wcNormal, orthoX); 
    vec3 diffuseColor = diffuseConeTrace(pos, wcNormal, 2.0f);
    diffuseColor += diffuseConeTrace(pos, mix(wcNormal, orthoX, 0.3), 2.0f);
    diffuseColor += diffuseConeTrace(pos, mix(wcNormal, -orthoX, 0.3), 2.0f);
    diffuseColor += diffuseConeTrace(pos, mix(wcNormal, orthoY, 0.3), 2.0f);
    diffuseColor += diffuseConeTrace(pos, mix(wcNormal, -orthoY, 0.3), 2.0f);
    vec3 view  = normalize(pos - camPosition.xyz);    
    vec3 specularColor = vec4(0.0f);
    if(shininess > 0.0f) {
        specularColor = specularConeTrace(pos, reflect(view, wcNormal), shininess);
    }

    FragColor = vec4(texture(texDiffuse, fTexCoord).rgb * (diffuseColor + specularColor), 1.0f);
}
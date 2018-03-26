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

vec3 InnerProduct(GaussianLobe g1, GaussianLobe g2) {
    float uLength = length(g1.sharpness * g1.axis + g2.sharpness * g2.axis);
    float eFactorized = exp(uLength - g1.sharpness - g2.sharpness);
    float factorized2 = 1.0f - exp(-2.0f * uLength);
    return (2.0f * PI * g1.amplitude * g2.amplitude * eFactorized * factorized2) / uLength;
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

bool isWithinBoundary(vec3 inputPos) {
    if(inputPos.x < level2min.x || inputPos.x > level2max.x
        || inputPos.y < level2min.y || inputPos.y > level2max.y
        || inputPos.z < level2min.z || inputPos.z > level2max.z) {
            return false;
    } 
    return true;
}
float evaluateLOD(float degree, float len) {
    return log2(2 * len * tan(radians(degree)));
}
vec4 evaluateColor(in sampler3D colorClip, in sampler3D normalClip, in sampler3D lightDirClip, 
    in usampler3D lightEnergyClip, float lod, vec3 clipPos, vec3 viewDir) {
        vec4 color = vec4(0.0f);
        float cosPhi = cos(radians(60));
        vec4 c = textureLod(colorClip, clipPos, lod);
        if(gl_FragCoord.x < 1.0f && gl_FragCoord.y < 1.0f) {
            logFragment(c, vec4(clipPos, lod), 0, 4, 2, 3);
            //logFragment(voxelToClipmapL0Mat[0], voxelToClipmapL0Mat[1], 0, 0, 0, 0);
            //logFragment(voxelToClipmapL0Mat[2], voxelToClipmapL0Mat[3], 0, 0, 0, 0);
        }
        //if(c.a > 0.0f) {                
            vec4 n = 2 * textureLod(normalClip, clipPos, lod) - 1.0f;
            uint lEnergy = textureLod(lightEnergyClip, clipPos, lod).r;
            vec4 l = 2 * textureLod(lightDirClip, clipPos, lod) - 1.0f;
            if(gl_FragCoord.x < 1.0f && gl_FragCoord.y < 1.0f) {
                logFragment(c, n, lEnergy, 2, 2, 3);
                logFragment(l, vec4(clipPos, 1.0f), lEnergy, 2, 2, 3);
            }

            GaussianLobe normalLobe = generateSG(vec3(1.0f), n.xyz);
            GaussianLobe lightLobe = generateSG(vec3(1.0f), l.xyz);
            
            GaussianLobe viewLobe;
            viewLobe.amplitude = vec3(1.0f);
            viewLobe.axis = viewDir;
            viewLobe.sharpness = 1.0f/(cosPhi * cosPhi);
            vec3 brdf = c.rgb / PI;
            vec3 convLightNormal = max(InnerProduct(normalLobe, lightLobe), 0.0f);

            color.rgb = brdf * float(lEnergy)/400000000.0f * convLightNormal;
            color.a = c.a;
        //}
        return color;
    }
// origin & dir in world space
vec3 diffuseConeTrace(vec3 origin, vec3 dir) {
    if(!isWithinBoundary(origin)) {
        return vec3(0.0);
    }
    float angle = 60.0f;
    float alpha = 0.0f;
    vec3 adjustedDir = pow(2, findMinLevel(origin)) * dir; // lengthen dir when traversing through larger grid dim
    vec3 rayVoxelPos = origin + adjustedDir;
    float lod; vec4 clipPos;

    vec3 color = vec3(0.0f);
    vec4 c;
    while(alpha < 1.0f && isWithinBoundary(rayVoxelPos)) {
        lod = evaluateLOD(30.0f, length(rayVoxelPos - origin));
        if(gl_FragCoord.x < 1.0f && gl_FragCoord.y < 1.0f) {
            logFragment(vec4(adjustedDir, 1.0f), vec4(rayVoxelPos, lod), uint(findMinLevel(rayVoxelPos)), 1, 1, 3);
        }
        adjustedDir = pow(2, int(lod)) * dir;
        if(lod < 1.0f) {
            clipPos = (voxelToClipmapL0Mat * vec4(rayVoxelPos, 1.0f));
            clipPos = clipPos/clipPos.w / 128.0f;
            c = evaluateColor(colorBrickL0, normalBrickL0, lightDirBrickL0, lightEnergyBrickL0, lod, clipPos.xyz, dir);
        } else if(lod < 2.0f) {
            clipPos = (voxelToClipmapL1Mat * vec4(rayVoxelPos, 1.0f));
            clipPos = clipPos/clipPos.w / 128.0f;
            lod = lod - 1.0f;
            c = evaluateColor(colorBrickL1, normalBrickL1, lightDirBrickL1, lightEnergyBrickL1, lod, clipPos.xyz, dir);
        } else {
            clipPos = (voxelToClipmapL2Mat * vec4(rayVoxelPos, 1.0f));
            clipPos = clipPos/clipPos.w / 128.0f;
            lod = lod - 2.0f;
            c = evaluateColor(colorBrickL2, normalBrickL2, lightDirBrickL2, lightEnergyBrickL2, lod, clipPos.xyz, dir);
        }
        color += c.rgb;
        alpha = alpha + (1.0f - alpha) * c.a;
        rayVoxelPos += adjustedDir;
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
    vec3 normal = normalize(wcNormal);
    vec3 orthoX = findOrthoVector(normal);
    vec3 orthoY = cross(normal, orthoX); 
    vec3 diffuseColor = diffuseConeTrace(pos, normal);
    diffuseColor += diffuseConeTrace(pos, mix(normal, orthoX, 0.4));
    diffuseColor += diffuseConeTrace(pos, mix(normal, -orthoX, 0.4));
    diffuseColor += diffuseConeTrace(pos, mix(normal, orthoY, 0.4));
    diffuseColor += diffuseConeTrace(pos, mix(normal, -orthoY, 0.4));
    vec3 view  = normalize(pos - camPosition.xyz);    
    vec3 specularColor = vec3(0.0f);
    /*if(shininess > 0.0f) {
        specularColor = specularConeTrace(pos, reflect(view, normal), shininess);
    }*/
    FragColor = vec4(texture(texDiffuse, fTexCoord).rgb * (diffuseColor + specularColor), 1.0f);
}
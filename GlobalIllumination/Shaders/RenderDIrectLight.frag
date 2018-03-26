#version 450
#extension GL_ARB_bindless_texture : require
#define PI           3.14159265358979323846
#define MAX_LIGHTS 1
#define LIGHT_FACTOR 1.0f

layout (location = 0) in vec3 wcPosition;
layout (location = 1) in vec3 ecNormal;    
layout (location = 2) in vec2 fTexCoord;

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
struct Light {
    vec4 lightPosition;
    vec4 lightEnergy;
};
layout(binding = 2) uniform LightBlocks {    
    Light lights[MAX_LIGHTS];
};

layout(binding = 0, std140) uniform MatrixBlock {
    mat4 ModelViewMatrix;     // ModelView matrix.
    mat4 ModelViewProjMatrix; // ModelView matrix * Projection matrix.
    mat3 NormalMatrix;        // For transforming wc to ec
    mat3 invNormalMatrix;
};
layout(binding = 2, std140) uniform ShadowMatrixBlock {
    mat4 ShadowMatrix[MAX_LIGHTS];
}
layout(binding=0) uniform sampler2DArray shadowMaps;          

const vec2 size = vec2(2.0,0.0);
const ivec3 off = ivec3(-1,0,1);

layout (location = 0) out vec4 FragColor;

void main()
{
    if(texture(texAlpha, fTexCoord).r < 0.5f) {
        discard;
    }
    vec3 viewVec = -normalize(ecPosition);
    vec3 necNormal = ecNormal;
    
    if(useBumpMap == 1) {            
        float h11 = texture(texHeight, fTexCoord).r;
        float h01 = textureOffset(texHeight, fTexCoord, off.xy).r;
        float h21 = textureOffset(texHeight, fTexCoord, off.zy).r;
        float h10 = textureOffset(texHeight, fTexCoord, off.yx).r;
        float h12 = textureOffset(texHeight, fTexCoord, off.yz).r;

        vec3 va = normalize(vec3(size.xy,h21-h01));
        vec3 vb = normalize(vec3(size.yx,h12-h10));

        necNormal = necNormal + cross(va, vb);
    }
    vec3 wcNormal = normalize(invNormalMatrix * necNormal);
    vec3 color = vec3(0.0f);
    for(int i = 0; i < MAX_LIGHTS; i++) {        
        vec3 lightVec;
        vec4 pos = lights[i].lightPosition;
        if (pos.w == 0.0 )
            lightVec = normalize(pos.xyz);
        else {
            lightVec = normalize(pos.xyz - wcPosition);
        }
        vec4 shadowCoord = ShadowMatrix[i] * vec4(wcPosition, 1.0f);
        float visibility = 1.0;
        if(texture(shadowMaps, vec3(shadowCoord.xy, float(i)).z < shadowCoord.z)) {
            visibility = 0.0;
        }
        vec3 reflectVec = reflect(-lightVec, wcNormal);
        float N_L = max(0.0, dot(wcNormal, lightVec));
        color +=  visibility * LIGHT_FACTOR * (diffuse.rgb * texture(texDiffuse, fTexCoord).rgb) / M_PI * lightEnergy.rgb * N_L;
    }
    FragColor = vec4(color, 1.0);
}
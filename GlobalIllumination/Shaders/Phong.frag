#version 450
#extension GL_ARB_bindless_texture : require

layout (location = 0) in vec3 ecPosition;   // Fragment's 3D position in eye space.
layout (location = 1) in vec3 ecNormal;     // Fragment's normal vector in eye space.
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

layout(binding = 2) uniform LightBlock {                            
    vec4 LightPosition; // Given in eye space. Can be directional.
    vec4 LightAmbient; 
    vec4 LightDiffuse;
    vec4 LightSpecular;
};

const vec2 size = vec2(2.0,0.0);
const ivec3 off = ivec3(-1,0,1);

layout (location = 0) out vec4 FragColor;

void main()
{
    if(texture(texAlpha, fTexCoord).r < 0.5f) {
        discard;
    }
    vec3 viewVec = -normalize(ecPosition);
    vec3 necNormal = normalize(ecNormal);

    vec3 lightVec;
    if (LightPosition.w == 0.0 )
        lightVec = normalize(LightPosition.xyz);
    else {
        lightVec = normalize(LightPosition.xyz - ecPosition);
    }
    
    if(useBumpMap == 1) {            
        float h11 = texture(texHeight, fTexCoord).r;
        float h01 = textureOffset(texHeight, fTexCoord, off.xy).r;
        float h21 = textureOffset(texHeight, fTexCoord, off.zy).r;
        float h10 = textureOffset(texHeight, fTexCoord, off.yx).r;
        float h12 = textureOffset(texHeight, fTexCoord, off.yz).r;

        vec3 va = normalize(vec3(size.xy,h21-h01));
        vec3 vb = normalize(vec3(size.yx,h12-h10));

        necNormal = normalize(necNormal + cross(va, vb));
    }

    vec3 reflectVec = reflect(-lightVec, necNormal);
    float N_L = max(0.0, dot(necNormal, lightVec));
    float R_V = max(0.0, dot(reflectVec, viewVec));
    float spec = (R_V == 0.0) ? 0.0 : pow(R_V, shininess);
    vec3 color = LightAmbient.rgb  * (ambient.rgb * texture(texAmbient, fTexCoord).rgb)
        + LightDiffuse.rgb * (diffuse.rgb * texture(texDiffuse, fTexCoord).rgb) * N_L 
        + LightSpecular.rgb * specular.rgb * spec;
    FragColor = vec4(color, 1.0);
}
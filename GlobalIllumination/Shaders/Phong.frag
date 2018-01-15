#version 450
#extension GL_ARB_bindless_texture : require

layout (location = 0) in vec3 ecPosition;   // Fragment's 3D position in eye space.
layout (location = 1) in vec3 ecNormal;     // Fragment's normal vector in eye space.
layout (location = 2) in vec2 fTexCoord;
layout (location = 3) flat in sampler2D texAmbient;
layout (location = 4) flat in sampler2D texDiffuse;
layout (location = 5) flat in sampler2D texAlpha;
layout (location = 6) flat in sampler2D texHeight;
layout (location = 7) flat in int useBumpMap;
layout (location = 8) flat in vec3 ambient;
layout (location = 9) flat in vec3 diffuse;
layout (location = 10) flat in vec3 specular;
layout (location = 11) flat in float shininess;

layout(binding = 0) uniform atomic_uint fragListPtr;

uniform mat4 ModelViewMatrix;     // ModelView matrix.
uniform mat4 ModelViewProjMatrix; // ModelView matrix * Projection matrix.
uniform mat3 NormalMatrix;        // For transforming object-space direction 
                                  //   vector to eye space.
                            
uniform vec4 LightPosition; // Given in eye space. Can be directional.
uniform vec4 LightAmbient; 
uniform vec4 LightDiffuse;
uniform vec4 LightSpecular;

/*
layout (binding=0) uniform sampler2D ambientTexture;
layout (binding=1) uniform sampler2D diffuseTexture;
layout (binding=2) uniform sampler2D alphaTexture;
layout (binding=3) uniform sampler2D heightTexture;
layout (binding=4) uniform sampler2D specTexture;

uniform vec3 Ambient; 
uniform vec3 Diffuse;
uniform vec3 Specular;
uniform float MatlShininess;
*/

//uniform bool useBumpMap;

const vec2 size = vec2(2.0,0.0);
const ivec3 off = ivec3(-1,0,1);

layout (location = 0) out vec4 FragColor;

void main()
{
    if(texture(texAlpha, fTexCoord).r < 0.5f) {
        discard;
    }
    atomicCounterIncrement(fragListPtr);
    vec3 viewVec = -normalize(ecPosition);
    vec3 necNormal = normalize(ecNormal);

    vec3 lightVec;
    if (LightPosition.w == 0.0 )
        lightVec = normalize(LightPosition.xyz);
    else
        lightVec = normalize(LightPosition.xyz - ecPosition);
    
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
    vec3 color = LightAmbient.rgb  * (ambient * texture(texAmbient, fTexCoord).rgb)
        + LightDiffuse.rgb * (diffuse * texture(texDiffuse, fTexCoord).rgb) * N_L 
        + LightSpecular.rgb * specular * spec;
    FragColor = vec4(color, 1.0);
}
#version 450

in vec3 ecPosition;   // Fragment's 3D position in eye space.
in vec3 ecNormal;     // Fragment's normal vector in eye space.
in vec2 fTexCoord;

uniform mat4 ModelViewMatrix;     // ModelView matrix.
uniform mat4 ModelViewProjMatrix; // ModelView matrix * Projection matrix.
uniform mat3 NormalMatrix;        // For transforming object-space direction 
                                  //   vector to eye space.
                            
uniform vec4 LightPosition; // Given in eye space. Can be directional.
uniform vec4 LightAmbient; 
uniform vec4 LightDiffuse;
uniform vec4 LightSpecular;

layout (binding=0) uniform sampler2D ambientTexture;
layout (binding=1) uniform sampler2D diffuseTexture;
layout (binding=2) uniform sampler2D alphaTexture;
layout (binding=3) uniform sampler2D heightTexture;
layout (binding=4) uniform sampler2D specTexture;

uniform vec3 Ambient; 
uniform vec3 Diffuse;
uniform vec3 Specular;
uniform float MatlShininess;

uniform bool useAlphaMap;
uniform bool useBumpMap;
uniform bool useSpecMap;

const vec2 size = vec2(2.0,0.0);
const ivec3 off = ivec3(-1,0,1);

layout (location = 0) out vec4 FragColor;

void main()
{
    if(useAlphaMap) {
        if(texture(alphaTexture, fTexCoord).r < 0.5f) {
            discard;
        }
    }
    vec3 viewVec = -normalize(ecPosition);
    vec3 necNormal = normalize(ecNormal);

    vec3 lightVec;
    if (LightPosition.w == 0.0 )
        lightVec = normalize(LightPosition.xyz);
    else
        lightVec = normalize(LightPosition.xyz - ecPosition);
    
    if(useBumpMap) {            
        float h11 = texture(heightTexture, fTexCoord).r;
        float h01 = textureOffset(heightTexture, fTexCoord, off.xy).r;
        float h21 = textureOffset(heightTexture, fTexCoord, off.zy).r;
        float h10 = textureOffset(heightTexture, fTexCoord, off.yx).r;
        float h12 = textureOffset(heightTexture, fTexCoord, off.yz).r;

        vec3 va = normalize(vec3(size.xy,h21-h01));
        vec3 vb = normalize(vec3(size.yx,h12-h10));

        necNormal = normalize(necNormal + cross(va, vb));
    }

    vec3 reflectVec = reflect(-lightVec, necNormal);
    float N_L = max(0.0, dot(necNormal, lightVec));
    float R_V = max(0.0, dot(reflectVec, viewVec));
    float spec = (R_V == 0.0) ? 0.0 : pow(R_V, MatlShininess);
    vec3 color = LightAmbient.rgb  * (Ambient * texture(ambientTexture, fTexCoord).rgb)
        + LightDiffuse.rgb * (Diffuse * texture(diffuseTexture, fTexCoord).rgb) * N_L 
        + LightSpecular.rgb * Specular * spec;
    FragColor = vec4(color, 1.0);
}
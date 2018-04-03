const vec2 size = vec2(2.0,0.0);
const ivec3 off = ivec3(-1,0,1);

layout (location = 0) out vec4 worldPosition;//change to worldPos using RGBA32F
layout (location = 1) out vec4 worldNormal;
layout (location = 2) out vec4 albedo;
layout (location = 3) out float spec;

void main()
{
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

    worldPosition = vec4(wcPosition, 1.0f);//RGBA32F
    worldNormal = vec4(nwcNormal, 1.0f) * 0.5f + 0.5f;//RGB10_A2
    albedo = diffuse * texture(texDiffuse, fTexCoord);//RGBA8
    spec = shininess;//R16F
    // ivec2 xy = ivec2(gl_FragCoord.xy);
    // if(xy.x < 1 && xy.y < 1) {
    //     logFragment(albedo, vec4(nwcNormal, 1.0f), 0, 1, 2, 3);
    // }
}
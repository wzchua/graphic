const vec2 size = vec2(2.0,0.0);
const ivec3 off = ivec3(-1,0,1);

layout (location = 0) out vec4 voxelPosition;//change to worldPos using RGBA32F
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

    vec4 pos = WorldToVoxelMat * vec4(wcPosition, 1.0f);
    pos = pos/pos.w;
    pos.xyz /= 512.0f;

    voxelPosition = pos;
    worldNormal = vec4(nwcNormal, 1.0f) * 0.5f + 0.5f;
    albedo = vec4(diffuse.rgb * texture(texDiffuse, fTexCoord).rgb, 1.0f);
    spec = shininess;
}
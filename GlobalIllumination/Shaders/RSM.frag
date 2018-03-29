layout (location = 0) out vec4 voxelPosition;
layout (location = 1) out vec4 worldNormal;
const float voxelDim = 512.0f;
const vec2 size = vec2(2.0,0.0);
const ivec3 off = ivec3(-1,0,1);
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

    vec4 voxelPos = WorldToVoxelMat * vec4(wcPosition, 1.0f);
    voxelPos.xyz /= voxelDim;
    voxelPosition = voxelPos;
    worldNormal = vec4(normalize(nwcNormal), 1.0f) * 0.5f + 0.5f;
}
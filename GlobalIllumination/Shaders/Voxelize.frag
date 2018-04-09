const vec2 size = vec2(2.0,0.0);
const ivec3 off = ivec3(-1,0,1);

void addToFragList(vec4 color, vec3 normal) {
    uint index = atomicAdd(fragmentCounter, 1);
    //uint index = atomicCounterIncrement(fragListPtr);
    if(index < maxNoOfFragments) {
        frag[index].position = vec4(wcPosition, 1.0f);
        frag[index].color = color;
        frag[index].normal = vec4(normal, 1.0f);
        //logFragment(vec4(wcPosition, 1.0f), color, 0, 0, 0, 0);
    } else {
        atomicAdd(fragmentCounter, uint(-1));
        //atomicCounterDecrement(fragListPtr);
    }
}

//Generates a voxel list from rasterization
void main() {    
    //if(fragmentCounter >= maxNoOfFragments) {
        //noOfFrag >= maxNoOfFragments
    //uint noOfFrag = atomicCounter(fragListPtr);
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
    vec4 color = vec4(diffuse.rgb * texture(texDiffuse, fTexCoord).rgb, 1.0f);
    addToFragList(color, nwcNormal);
}
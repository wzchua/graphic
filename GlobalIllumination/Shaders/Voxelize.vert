void main()
{
    vec4 wcPosition4 = WorldToVoxelMat * vec4(vPosition, 1.0f);
    geomwcPosition = vec3( wcPosition4 ) / wcPosition4.w;
    geomwcNormal = vNormal; //scale is done 1:1:1 ratio
    geomTexCoord = vTexCoord;
    gl_Position = ViewProjMatrixXY * WorldToVoxelMat * vec4(vPosition, 1.0f);
}
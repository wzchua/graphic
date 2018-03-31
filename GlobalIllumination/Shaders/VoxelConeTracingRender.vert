void main()
{    
    wcPosition = vPosition;
    wcNormal = vNormal;
    fTexCoord = vTexCoord;
    gl_Position = ModelViewProjMatrix * vec4(vPosition, 1.0);
}
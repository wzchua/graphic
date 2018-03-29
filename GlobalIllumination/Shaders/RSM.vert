void main()
{    
    wcPosition = vPosition;
    wcNormal = vNormal;
    gl_Position = ModelViewProjMatrix * vec4(vPosition, 1.0);
}
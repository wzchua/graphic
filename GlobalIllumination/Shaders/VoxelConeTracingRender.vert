layout(binding = 0, std140) uniform MatrixBlock {
    mat4 ModelViewMatrix;     // ModelView matrix.
    mat4 ModelViewProjMatrix; // ModelView matrix * Projection matrix.
    mat3 NormalMatrix;        // For transforming object-space direction 
};                                    //   vector to eye space.

void main()
{    
    wcPosition = vPosition;
    wcNormal = vNormal;
    gl_Position = ModelViewProjMatrix * vec4(vPosition, 1.0);
}
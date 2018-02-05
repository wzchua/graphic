// adapted from https://github.com/Friduric/voxel-cone-tracing/blob/master/Shaders/Voxelization/voxelization.geom

#version 450 core
#extension GL_ARB_bindless_texture : require

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 geomwcPosition[];
in vec3 geomwcNormal[];
in vec2 geomTexCoord[];

layout(binding = 0) uniform VoxelizeMatrixBlock {
    mat4 WorldToVoxelMat;
    mat4 ViewProjMatrixXY; 
    mat4 ViewProjMatrixZY; 
    mat4 ViewProjMatrixXZ;
};

out vec3 wcPosition;
out vec3 wcNormal;
out vec2 fTexCoord;

void main() {
    vec3 triangleNormal = normalize( cross( geomwcPosition[1]-geomwcPosition[0], geomwcPosition[2]-geomwcPosition[0] ) );
    triangleNormal = abs(triangleNormal);
    /*
    if(projectionAxis == 0) {
        axis = 1;
        for(int i = 0; i < 3; i++) {
            wcPosition = geomwcPosition[i];
            wcNormal = geomwcNormal[i];
            fTexCoord = geomTexCoord[i];
            gl_Position = vec4(gl_in[i].gl_Position.x, gl_in[i].gl_Position.y, gl_in[i].gl_Position.z, 1.0f);
            EmitVertex();
        }
        EndPrimitive();
    }
    if(projectionAxis == 1) {
        axis = 2;
        for(int i = 0; i < 3; i++) {
            wcPosition = geomwcPosition[i];
            wcNormal = geomwcNormal[i];
            fTexCoord = geomTexCoord[i];
            gl_Position = vec4(gl_in[i].gl_Position.z, gl_in[i].gl_Position.y, gl_in[i].gl_Position.x, 1.0f);
            EmitVertex();
        }
        EndPrimitive();
    }
    if(projectionAxis == 2) {
        axis = 3;
        for(int i = 0; i < 3; i++) {
            wcPosition = geomwcPosition[i];
            wcNormal = geomwcNormal[i];
            fTexCoord = geomTexCoord[i];
            gl_Position = vec4(gl_in[i].gl_Position.x, gl_in[i].gl_Position.z, gl_in[i].gl_Position.y, 1.0f);
            EmitVertex();
        }
        EndPrimitive();
    }
    */

    
    if(triangleNormal.x > triangleNormal.y && triangleNormal.x > triangleNormal.z) {
        for(int i = 0; i < 3; i++) {
            wcPosition = geomwcPosition[i];
            wcNormal = geomwcNormal[i];
            fTexCoord = geomTexCoord[i];
            gl_Position = vec4(gl_in[i].gl_Position.z, gl_in[i].gl_Position.y, gl_in[i].gl_Position.x, 1.0f);
            EmitVertex();
        }
        EndPrimitive();
    } else if(triangleNormal.z > triangleNormal.x && triangleNormal.z > triangleNormal.y) {
        for(int i = 0; i < 3; i++) {
            wcPosition = geomwcPosition[i];
            wcNormal = geomwcNormal[i];
            fTexCoord = geomTexCoord[i];
            gl_Position = vec4(gl_in[i].gl_Position.x, gl_in[i].gl_Position.y, gl_in[i].gl_Position.z, 1.0f);
            EmitVertex();
        }
        EndPrimitive();
    } else {        
        for(int i = 0; i < 3; i++) {
            wcPosition = geomwcPosition[i];
            wcNormal = geomwcNormal[i];
            fTexCoord = geomTexCoord[i];
            gl_Position = vec4(gl_in[i].gl_Position.x, gl_in[i].gl_Position.z, gl_in[i].gl_Position.y, 1.0f);
            EmitVertex();
        }
        EndPrimitive();
    }
}
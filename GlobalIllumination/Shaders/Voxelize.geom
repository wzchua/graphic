// adapted from https://github.com/Friduric/voxel-cone-tracing/blob/master/Shaders/Voxelization/voxelization.geom

#version 450 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 9) out;


in vec3 vwcPosition[];
in vec3 vwcNormal[];
in vec2 vTexCoord[];

uniform mat4 ViewProjMatrixXY; 
uniform mat4 ViewProjMatrixZY; 
uniform mat4 ViewProjMatrixXZ;

uniform int projectionAxis;

out vec3 wcPosition;
out vec3 wcNormal;
out vec2 fTexCoord;
out int axis;

void main() {
    vec3 triangleNormal = normalize( cross( vwcPosition[1]-vwcPosition[0], vwcPosition[2]-vwcPosition[0] ) );
    triangleNormal = abs(triangleNormal);
    //if(projectionAxis == 0) {
        axis = 1;
        for(int i = 0; i < 3; i++) {
            wcPosition = vwcPosition[i];
            wcNormal = vwcNormal[i];
            fTexCoord = vTexCoord[i];
            gl_Position = ViewProjMatrixXY * gl_in[i].gl_Position;
            EmitVertex();
        }
        EndPrimitive();
    //}
    //if(projectionAxis == 1) {
        axis = 2;
        for(int i = 0; i < 3; i++) {
            wcPosition = vwcPosition[i];
            wcNormal = vwcNormal[i];
            fTexCoord = vTexCoord[i];
            gl_Position = ViewProjMatrixZY * gl_in[i].gl_Position;
            EmitVertex();
        }
        EndPrimitive();
    //}
    //if(projectionAxis == 2) {
        axis = 3;
        for(int i = 0; i < 3; i++) {
            wcPosition = vwcPosition[i];
            wcNormal = vwcNormal[i];
            fTexCoord = vTexCoord[i];
            gl_Position = ViewProjMatrixXZ * gl_in[i].gl_Position;
            EmitVertex();
        }
        EndPrimitive();
    //}

    /*
    if(triangleNormal.x > triangleNormal.y && triangleNormal.x > triangleNormal.z) {
        for(int i = 0; i < 3; i++) {
            wcPosition = vwcPosition[i];
            wcNormal = vwcNormal[i];
            fTexCoord = vTexCoord[i];
            gl_Position = vec4(gl_in[i].gl_Position.z, gl_in[i].gl_Position.y, gl_in[i].gl_Position.x, 1.0f);
            EmitVertex();
        }
        EndPrimitive();
    } else if(triangleNormal.z > triangleNormal.x && triangleNormal.z > triangleNormal.y) {
        for(int i = 0; i < 3; i++) {
            wcPosition = vwcPosition[i];
            wcNormal = vwcNormal[i];
            fTexCoord = vTexCoord[i];
            gl_Position = vec4(gl_in[i].gl_Position.x, gl_in[i].gl_Position.y, gl_in[i].gl_Position.z, 1.0f);
            EmitVertex();
        }
        EndPrimitive();
    } else {        
        for(int i = 0; i < 3; i++) {
            wcPosition = vwcPosition[i];
            wcNormal = vwcNormal[i];
            fTexCoord = vTexCoord[i];
            gl_Position = vec4(gl_in[i].gl_Position.x, gl_in[i].gl_Position.z, gl_in[i].gl_Position.y, 1.0f);
            EmitVertex();
        }
        EndPrimitive();
    }*/
}
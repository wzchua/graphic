// adapted from https://github.com/Friduric/voxel-cone-tracing/blob/master/Shaders/Voxelization/voxelization.geom

#version 450 core

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;


in vec3 vwcPosition[];
in vec3 vwcNormal[];
in vec2 vTexCoord[];

out vec3 wcPosition;
out vec3 wcNormal;
out vec2 fTexCoord;

void main() {
    vec3 triangleNormal = normalize( cross( vwcPosition[1]-vwcPosition[0], vwcPosition[2]-vwcPosition[0] ) );
    triangleNormal = abs(triangleNormal);
    if(triangleNormal.x > triangleNormal.y && triangleNormal.x > triangleNormal.z) {
        for(int i = 0; i < 3; i++) {
            wcPosition = vwcPosition[i];
            wcNormal = vwcNormal[i];
            fTexCoord = vTexCoord[i];
            gl_Position = vec4(gl_in[i].gl_Position.z, gl_in[i].gl_Position.y, 0.0f, 1.0f);
            EmitVertex();
        }
    } else if(triangleNormal.z > triangleNormal.x && triangleNormal.z > triangleNormal.y) {
        for(int i = 0; i < 3; i++) {
            wcPosition = vwcPosition[i];
            wcNormal = vwcNormal[i];
            fTexCoord = vTexCoord[i];
            gl_Position = vec4(gl_in[i].gl_Position.x, gl_in[i].gl_Position.y, 0.0f, 1.0f);
            EmitVertex();
        }
    } else {        
        for(int i = 0; i < 3; i++) {
            wcPosition = vwcPosition[i];
            wcNormal = vwcNormal[i];
            fTexCoord = vTexCoord[i];
            gl_Position = vec4(gl_in[i].gl_Position.x, gl_in[i].gl_Position.z, 0.0f, 1.0f);
            EmitVertex();
        }
    }
    EndPrimitive();
}
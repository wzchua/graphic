#version 450 core

layout(points) in;
layout(triangle_strip, max_vertices = 36) out;

uniform mat4 ModelViewProjMat;

out flat ivec3 voxelPos;

void main() {
    vec3 pos = gl_in[0].gl_Position.xyz;

    //+x    
    gl_Position = ModelViewProjMat * vec4(pos + vec3(0.5, -0.5, -0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    gl_Position = ModelViewProjMat * vec4(pos + vec3(0.5, 0.5, 0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    gl_Position = ModelViewProjMat * vec4(pos + vec3(0.5, 0.5, -0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    EndPrimitive();

    gl_Position = ModelViewProjMat * vec4(pos + vec3(0.5, -0.5, -0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    gl_Position = ModelViewProjMat * vec4(pos + vec3(0.5, -0.5, 0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    gl_Position = ModelViewProjMat * vec4(pos + vec3(0.5, 0.5, 0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    EndPrimitive();

    //-x
    gl_Position = ModelViewProjMat * vec4(pos + vec3(-0.5, -0.5, 0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    gl_Position = ModelViewProjMat * vec4(pos + vec3(-0.5, 0.5, -0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    gl_Position = ModelViewProjMat * vec4(pos + vec3(-0.5, 0.5, 0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    EndPrimitive();

    gl_Position = ModelViewProjMat * vec4(pos + vec3(-0.5, -0.5, 0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    gl_Position = ModelViewProjMat * vec4(pos + vec3(-0.5, -0.5, -0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    gl_Position = ModelViewProjMat * vec4(pos + vec3(-0.5, 0.5, -0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    EndPrimitive();
    
    //+y    
    gl_Position = ModelViewProjMat * vec4(pos + vec3(-0.5, 0.5, 0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    gl_Position = ModelViewProjMat * vec4(pos + vec3(0.5, 0.5, -0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    gl_Position = ModelViewProjMat * vec4(pos + vec3(-0.5, 0.5, -0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    EndPrimitive();

    gl_Position = ModelViewProjMat * vec4(pos + vec3(-0.5, 0.5, 0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    gl_Position = ModelViewProjMat * vec4(pos + vec3(0.5, 0.5, 0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    gl_Position = ModelViewProjMat * vec4(pos + vec3(0.5, 0.5, -0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    EndPrimitive();

    //-y
    gl_Position = ModelViewProjMat * vec4(pos + vec3(-0.5, -0.5, -0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    gl_Position = ModelViewProjMat * vec4(pos + vec3(0.5, -0.5, 0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    gl_Position = ModelViewProjMat * vec4(pos + vec3(-0.5, -0.5, 0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    EndPrimitive();

    gl_Position = ModelViewProjMat * vec4(pos + vec3(-0.5, -0.5, -0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    gl_Position = ModelViewProjMat * vec4(pos + vec3(0.5, -0.5, -0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    gl_Position = ModelViewProjMat * vec4(pos + vec3(0.5, -0.5, 0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    EndPrimitive();

    //+z    
    gl_Position = ModelViewProjMat * vec4(pos + vec3(-0.5, -0.5, 0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    gl_Position = ModelViewProjMat * vec4(pos + vec3(0.5, 0.5, 0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    gl_Position = ModelViewProjMat * vec4(pos + vec3(-0.5, 0.5, 0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    EndPrimitive();

    gl_Position = ModelViewProjMat * vec4(pos + vec3(-0.5, -0.5, 0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    gl_Position = ModelViewProjMat * vec4(pos + vec3(0.5, -0.5, 0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    gl_Position = ModelViewProjMat * vec4(pos + vec3(0.5, 0.5, 0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    EndPrimitive();

    //-z
    gl_Position = ModelViewProjMat * vec4(pos + vec3(0.5, -0.5, -0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    gl_Position = ModelViewProjMat * vec4(pos + vec3(-0.5, 0.5, -0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    gl_Position = ModelViewProjMat * vec4(pos + vec3(0.5, 0.5, -0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    EndPrimitive();

    gl_Position = ModelViewProjMat * vec4(pos + vec3(0.5, -0.5, -0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    gl_Position = ModelViewProjMat * vec4(pos + vec3(-0.5, -0.5, -0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    gl_Position = ModelViewProjMat * vec4(pos + vec3(-0.5, 0.5, -0.5), 1.0f);
    voxelPos = ivec3(pos);
    EmitVertex();
    EndPrimitive();
}
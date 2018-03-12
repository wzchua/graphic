#pragma once
#include <string>
namespace GenericShaderCodeString {
    static std::string vertHeader = R"(#version 450
#extension GL_ARB_bindless_texture : require

layout (location = 0) in vec3 vPosition;  // Vertex position in object space.
layout (location = 1) in vec3 vNormal;    // Vertex normal in object space.
layout (location = 2) in vec2 vTexCoord;    // Vertex normal in object space.
)";

    static std::string vertGeomOutput = R"(out vec3 geomwcPosition;   // Vertex position in scaled world space.
out vec3 geomwcNormal;     // Vertex normal in world space.
out vec2 geomTexCoord;
)";

    static std::string geomHeader = R"(#version 450 core
#extension GL_ARB_bindless_texture : require

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 geomwcPosition[];
in vec3 geomwcNormal[];
in vec2 geomTexCoord[];
)";

    static std::string geomFragOutput = R"(out vec3 wcPosition;
out vec3 wcNormal;
out vec2 fTexCoord;
)";

    static std::string fragHeader = R"(#version 450
#extension GL_ARB_bindless_texture : require

in vec3 wcPosition;   // Vertex position in scaled world space.
in vec3 wcNormal;     // Vertex normal in world space.
in vec2 fTexCoord;
)";

    static std::string genericLimitsUniformBlock(int level) {
        std::string s = R"*(layout(binding = )*" + std::to_string(level);
        s = s + R"*(, std140) uniform LimitsUniformBlock{
    uint maxNoOfFragments;
    uint maxNoOfNodes;
    uint maxNoOfBricks;
    uint maxNoOfLogs;
};
)*";
        return s;
    }

    static std::string materialUniformBlock(int level) {
        std::string s = R"(layout(binding = )" + level;
        s = s + R"() uniform MatBlock{
    sampler2D texAmbient;
    sampler2D texDiffuse;
    sampler2D texAlpha;
    sampler2D texHeight;
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    int useBumpMap;
    float shininess;
};
)";
        return s;
    }
}

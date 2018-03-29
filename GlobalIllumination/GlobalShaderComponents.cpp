#include "GlobalShaderComponents.h"



std::string GlobalShaderComponents::getHeader()
{
    return R"(#version 450
#extension GL_ARB_bindless_texture : require
)";
}

std::string GlobalShaderComponents::getVertTripleInputs()
{
    return R"(layout (location = 0) in vec3 vPosition;  // Vertex position in object space.
layout (location = 1) in vec3 vNormal;    // Vertex normal in object space.
layout (location = 2) in vec2 vTexCoord;    // Vertex normal in object space.
)";
}

std::string GlobalShaderComponents::getVertToGeomTripleOutput()
{
    return R"(out vec3 geomwcPosition;   // Vertex position in scaled world space.
out vec3 geomwcNormal;     // Vertex normal in world space.
out vec2 geomTexCoord;
)";
}

std::string GlobalShaderComponents::getVertToFragTripleOutput()
{
    return R"(out vec3 wcPosition;
out vec3 wcNormal;
out vec2 fTexCoord;
)";
}

std::string GlobalShaderComponents::getGeomTripleInput()
{
    return R"(layout(triangles) in;

in vec3 geomwcPosition[];
in vec3 geomwcNormal[];
in vec2 geomTexCoord[];
)";
}

std::string GlobalShaderComponents::getGeomToFragTripleOutput()
{
    return R"(layout(triangle_strip, max_vertices = 3) out;
out vec3 wcPosition;
out vec3 wcNormal;
out vec2 fTexCoord;
)";
}

std::string GlobalShaderComponents::getFragTripleInput()
{
    return R"(in vec3 wcPosition;   // Vertex position in scaled world space.
in vec3 wcNormal;     // Vertex normal in world space.
in vec2 fTexCoord;
)";
}

std::string GlobalShaderComponents::getComputeShaderInputLayout(GLuint x, GLuint y, GLuint z)
{
    return "layout (local_size_x = " + std::to_string(x) + ", local_size_y = " + std::to_string(y) + ", local_size_z = "+ std::to_string(z) +") in;\n";
}

std::string GlobalShaderComponents::getCamMatrixUBOCode()
{
    std::string s = R"*(layout(binding = )*" + std::to_string(CAMERA_MATRIX_UBO_BINDING)
        + R"*(, std140) uniform MatrixBlock{
    mat4 ModelViewMatrix;     // ModelView matrix.
    mat4 ModelViewProjMatrix; // ModelView matrix * Projection matrix.
    mat3 NormalMatrix;        // For transforming object-space direction 
};                                    //   vector to eye space.
)*";
    return s;
}
std::string GlobalShaderComponents::getLightCamMatrixUBOCode()
{
    std::string s = R"*(layout(binding = )*" + std::to_string(LIGHT_CAMERA_MATRIX_UBO_BINDING)
        + R"*(, std140) uniform MatrixBlock{
    mat4 ModelViewMatrix;     // ModelView matrix.
    mat4 ModelViewProjMatrix; // ModelView matrix * Projection matrix.
    mat3 NormalMatrix;        // For transforming object-space direction 
};                                    //   vector to eye space.
)*";
    return s;
}

std::string GlobalShaderComponents::getGlobalVariablesUBOCode()
{
    std::string s = R"*(layout(binding = )*" + std::to_string(GLOBAL_VARIABLES_UBO_BINDING)
     + R"*(, std140) uniform GlobalsUniformBlock{
    uint maxNoOfFragments;
    uint maxNoOfNodes;
    uint maxNoOfBricks;
    uint maxNoOfLogs;
};
)*";
    return s;
}

std::string GlobalShaderComponents::getMaterialUBOCode()
{
    std::string s = R"(layout(binding = )" + std::to_string(MATERIAL_UBO_BINDING)
    + R"() uniform MatBlock{
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

void GlobalShaderComponents::initializeUBO(GLuint & uboId, GLsizeiptr size, const void * data, GLenum usage)
{
    glGenBuffers(1, &uboId);
    glBindBuffer(GL_UNIFORM_BUFFER, uboId);
    glBufferData(GL_UNIFORM_BUFFER, size, data, usage);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GlobalShaderComponents::bindUBO(GLuint uboId, UBOType type)
{
    glBindBufferBase(GL_UNIFORM_BUFFER, type, uboId);
}


GlobalShaderComponents::GlobalShaderComponents()
{
}


GlobalShaderComponents::~GlobalShaderComponents()
{
}

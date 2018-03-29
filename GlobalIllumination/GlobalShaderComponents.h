#pragma once
#include <glad\glad.h>
#include <string>

class GlobalShaderComponents
{
public:
    static const GLuint CAMERA_MATRIX_UBO_BINDING = 0;
    static const GLuint MATERIAL_UBO_BINDING = 1;
    static const GLuint VOXELIZATION_MATRIX_UBO_BINDING = 2;
    static const GLuint GLOBAL_VARIABLES_UBO_BINDING = 3;
    static const GLuint CASGRID_VOXELIZATION_MATRIX_UBO_BINDING = 4;
    static const GLuint LIGHT_UBO_BINDING = 5;
    static const GLuint CAMERA_UBO_BINDING = 6;
    static const GLuint LIGHT_CAMERA_MATRIX_UBO_BINDING = 7;
    static const GLuint CAS_VOXELIZATION_MATRIX_UBO_BINDING = 8;
    
    static const GLuint COUNTER_SSBO_BINDING = 0;
    static const GLuint LOG_SSBO_BINDING = 1;
    static const GLuint OCTREE_NODE_SSBO_BINDING = 2;
    static const GLuint FRAGMENT_LIST_SSBO_BINDING = 3;
    static const GLuint FRAGMENT_LIST_SECONDARY_SSBO_BINDING = 4;

    static const GLuint COLOR_IMAGE_BINDING = 0;
    static const GLuint NORMAL_IMAGE_BINDING = 1;
    static const GLuint LIGHT_DIR_IMAGE_BINDING = 2;
    static const GLuint LIGHT_ENERGY_IMAGE_BINDING = 3;
    static const GLuint COLOR2_IMAGE_BINDING = 4;
    static const GLuint NORMAL2_IMAGE_BINDING = 5;
    static const GLuint LIGHT_DIR2_IMAGE_BINDING = 6;
    static const GLuint LIGHT_ENERGY2_IMAGE_BINDING = 7;
    static const GLuint COLOR3_IMAGE_BINDING = 8;
    static const GLuint NORMAL3_IMAGE_BINDING = 9;
    static const GLuint LIGHT_DIR3_IMAGE_BINDING = 10;
    static const GLuint LIGHT_ENERGY3_IMAGE_BINDING = 11;

    static std::string getHeader();
    static std::string getVertTripleInputs();
    static std::string getVertToGeomTripleOutput();
    static std::string getVertToFragTripleOutput();
    static std::string getGeomTripleInput();
    static std::string getGeomToFragTripleOutput();
    static std::string getFragTripleInput();

    static std::string getCamMatrixUBOCode();
    static std::string getLightCamMatrixUBOCode();
    static std::string getGlobalVariablesUBOCode();
    static std::string getMaterialUBOCode();
    enum UBOType {
        CAMERA_MATRIX = 0,
        MATERIAL = 1,
        VOXELIZATION_MATRIX = 2,
        GLOBAL_VARIABLES = 3,
        CASGRID_VOXELIZATION_MATRIX = 4,
        LIGHT = 5,
        CAMERA = 6,
        LIGHT_CAMERA = 7
    };
    static void initializeUBO(GLuint& uboId, GLsizeiptr size, const void* data, GLenum usage);
    static void bindUBO(GLuint uboId, UBOType type);

    struct GlobalsBlock {
        GLuint maxNoOfFragments;
        GLuint maxNoOfNodes;
        GLuint maxNoOfBricks;
        GLuint maxNoOfLogs;
    };
    GlobalShaderComponents();
    ~GlobalShaderComponents();
};


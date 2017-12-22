#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "GLBufferObject.h"
#include "ShaderProgram.h"
class FragmentToGrid
{
private:
    struct logStruct {
        float position[4];
        float color[4];
    };
    unsigned int maxLogCount = 500;
    unsigned int maxVoxelCount = 512 * 512 * 512;
    bool hasInitialized = false;

    GLuint gridDim = 512;

    //working memory
    GLBufferObject atomicVoxelCounter;
    GLBufferObject atomicLogCounter;
    GLBufferObject ssboVoxelList;
    GLBufferObject ssboLogList;
    GLuint texture3DrgColorBrickList;
    GLuint texture3DbaColorBrickList;
    GLuint texture3DxyNormalBrickList;
    GLuint texture3DzwNormalBrickList;
    GLuint texture3DCounterList;

    //output texture
    GLuint texture3DColorListRef;
    GLuint texture3DNormalListRef;

    ShaderProgram shaderBuildGrid;
    ShaderProgram shaderAverageGrid;
public:
    void initialize(GLuint& texture3DColorList, GLuint& texture3DNormalList);
    void run(GLBufferObject& inputssboFragmentList, GLuint noOfFragments);
    FragmentToGrid();
    ~FragmentToGrid();
};


#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "GLBufferObject.h"
#include "ShaderProgram.h"
#include "LogStruct.h"
#include "FragStruct.h"
#include "Scene.h"
#include "CounterBlock.h"
class RenderToGrid
{
private:
    struct logStruct {
        float position[4];
        float color[4];
    };

    unsigned int maxLogCount = 500;
    unsigned int maxVoxelCount = 512 * 512 * 512;
    bool hasInitialized = false;

    const std::vector<std::string> shaderDefiniations = { "GRID" };
    GLuint gridDim = 512;

    //working memory
    GLBufferObject<GLuint> atomicVoxelCounter;
    GLBufferObject<glm::ivec4> ssboVoxelList;
    GLuint texture3DrgColorBrickList;
    GLuint texture3DbaColorBrickList;
    GLuint texture3DxyNormalBrickList;
    GLuint texture3DzwNormalBrickList;
    GLuint texture3DCounterList;

    ShaderProgram voxelizeGridShader;
    ShaderProgram shaderAverageGrid;
public:
    void initialize();
    void run(Scene& inputScene, GLBufferObject<CounterBlock> & ssboCounterSet, GLuint voxelizeMatrixBlock, GLuint logUniformBlock, GLBufferObject<LogStruct> & ssboLogList, GLuint texture3DColor, GLuint texture3DNormal);
    RenderToGrid();
    ~RenderToGrid();
};


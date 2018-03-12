#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>

#include "GenericShaderCodeString.h"
#include "VoxelizeBlock.h"
#include "GLBufferObject.h"
#include "ShaderProgram.h"
#include "LogStruct.h"
#include "FragStruct.h"
#include "Scene.h"
#include "CounterBlock.h"
class RenderToGrid
{
private:
    unsigned int maxLogCount = 500;
    unsigned int maxVoxelCount = 512 * 512 * 512;
    bool hasInitialized = false;

    GLuint gridDim = 512;

    GLuint texture3DCounterList;

    ShaderProgram voxelizeGridShader;
public:
    void initialize();
    void run(Scene& inputScene, GLBufferObject<CounterBlock> & ssboCounterSet, GLuint voxelizeMatrixBlock, GLuint logUniformBlock, GLBufferObject<LogStruct> & ssboLogList, GLuint texture3DColor, GLuint texture3DNormal, GLBufferObject<glm::vec4> & voxelList);
    void resetData();
    RenderToGrid();
    ~RenderToGrid();
};


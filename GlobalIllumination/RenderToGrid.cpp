#include "RenderToGrid.h"



void RenderToGrid::initialize()
{
    if (hasInitialized) {
        return;
    }
    voxelizeGridShader.generateShader("./Shaders/Voxelize.vert", ShaderProgram::VERTEX);
    voxelizeGridShader.generateShader("./Shaders/Voxelize.geom", ShaderProgram::GEOMETRY);
    voxelizeGridShader.generateShader("./Shaders/VoxelizeGrid.frag", ShaderProgram::FRAGMENT);
    voxelizeGridShader.linkCompileValidate();

    glGenTextures(1, &texture3DCounterList);
    glBindTexture(GL_TEXTURE_3D, texture3DCounterList);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, gridDim, gridDim, gridDim);
    glBindTexture(GL_TEXTURE_3D, 0);
}

void RenderToGrid::run(Scene& inputScene, GLBufferObject<CounterBlock> & ssboCounterSet, GLuint voxelizeMatrixBlock, GLuint logUniformBlock, GLBufferObject<LogStruct> & ssboLogList, GLuint texture3DColor, GLuint texture3DNormal, GLBufferObject<glm::vec4> & voxelList)
{
    GLuint currentShaderProgram = voxelizeGridShader.use();
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, voxelizeMatrixBlock);
    glBindBufferBase(GL_UNIFORM_BUFFER, 7, logUniformBlock);

    ssboCounterSet.bind(1);
    voxelList.bind(2);
    ssboLogList.bind(7);

    glBindImageTexture(4, texture3DColor, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(5, texture3DNormal, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(7, texture3DCounterList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

    glViewport(0, 0, 512, 512);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    inputScene.render(currentShaderProgram);
    glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
}

void RenderToGrid::resetData()
{
    glInvalidateTexImage(texture3DCounterList, 0);
    glClearTexImage(texture3DCounterList, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL);
}

RenderToGrid::RenderToGrid()
{
}


RenderToGrid::~RenderToGrid()
{
}

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

    shaderAverageGrid.generateShader("./Shaders/AverageGrid.comp", ShaderProgram::COMPUTE);
    shaderAverageGrid.linkCompileValidate();

    glGenTextures(1, &texture3DrgColorBrickList);
    glBindTexture(GL_TEXTURE_3D, texture3DrgColorBrickList);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, gridDim, gridDim, gridDim);
    glBindTexture(GL_TEXTURE_3D, 0);

    glGenTextures(1, &texture3DbaColorBrickList);
    glBindTexture(GL_TEXTURE_3D, texture3DbaColorBrickList);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, gridDim, gridDim, gridDim);
    glBindTexture(GL_TEXTURE_3D, 0);

    glGenTextures(1, &texture3DxyNormalBrickList);
    glBindTexture(GL_TEXTURE_3D, texture3DxyNormalBrickList);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, gridDim, gridDim, gridDim);
    glBindTexture(GL_TEXTURE_3D, 0);

    glGenTextures(1, &texture3DzwNormalBrickList);
    glBindTexture(GL_TEXTURE_3D, texture3DzwNormalBrickList);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexStorage3D(GL_TEXTURE_3D, 1, GL_R32UI, gridDim, gridDim, gridDim);
    glBindTexture(GL_TEXTURE_3D, 0);

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

void RenderToGrid::run(Scene& inputScene, GLBufferObject<CounterBlock> & ssboCounterSet, GLuint voxelizeMatrixBlock, GLuint logUniformBlock, GLBufferObject<LogStruct> & ssboLogList, GLuint texture3DColor, GLuint texture3DNormal)
{
    GLuint currentShaderProgram = voxelizeGridShader.use();
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, voxelizeMatrixBlock);
    glBindBufferBase(GL_UNIFORM_BUFFER, 7, logUniformBlock);

    ssboCounterSet.bind(1);
    ssboLogList.bind(7);

    glBindImageTexture(0, texture3DrgColorBrickList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(1, texture3DbaColorBrickList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(2, texture3DxyNormalBrickList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(3, texture3DzwNormalBrickList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(7, texture3DCounterList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

    glViewport(0, 0, 512, 512);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    inputScene.render(currentShaderProgram);
    glMemoryBarrier(GL_ATOMIC_COUNTER_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

    currentShaderProgram = shaderAverageGrid.use();
    glBindImageTexture(4, texture3DColor, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
    glBindImageTexture(5, texture3DNormal, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);

    int workgroupX = 512 / 8;
    glDispatchCompute(workgroupX, workgroupX, workgroupX);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_ATOMIC_COUNTER_BARRIER_BIT);
    /*
    auto set = ssboCounterSet.getPtr();
    unsigned int fragmentCount = set[0].fragmentCounter;
    unsigned int logCount = set[0].logCounter;
    ssboCounterSet.unMapPtr();
    */
    //std::vector<LogStruct> logs;
    //ShaderLogger::getLogs(ssboLogList, logCount, logs);
}

RenderToGrid::RenderToGrid()
{
}


RenderToGrid::~RenderToGrid()
{
}

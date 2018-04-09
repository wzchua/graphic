#include "RenderToGrid.h"


typedef GlobalShaderComponents GlobalCom;

void RenderToGrid::initialize()
{
    if (hasInitialized) {
        return;
    }
    std::stringstream vertShaderString;
    vertShaderString << GlobalCom::getHeader() << GlobalCom::getVertTripleInputs() << GlobalCom::getVertToGeomTripleOutput();
    vertShaderString << GlobalCom::getGlobalVariablesUBOCode();
    vertShaderString << voxelizeBlockString(GlobalCom::VOXELIZATION_MATRIX_UBO_BINDING) << counterBlockBufferShaderCodeString(GlobalCom::COUNTER_SSBO_BINDING) << logFunctionAndBufferShaderCodeString(GlobalCom::LOG_SSBO_BINDING);

    voxelizeGridShader.generateShader(vertShaderString, "./Shaders/Voxelize.vert", ShaderProgram::VERTEX);
    std::stringstream geomShaderString;
    geomShaderString << GlobalCom::getHeader() << GlobalCom::getGeomTripleInput() << GlobalCom::getGeomToFragTripleOutput();
    voxelizeGridShader.generateShader(geomShaderString, "./Shaders/Voxelize.geom", ShaderProgram::GEOMETRY);

    std::stringstream fragShaderString;
    fragShaderString << GlobalCom::getHeader() << GlobalCom::getFragTripleInput();
    fragShaderString << GlobalCom::getMaterialUBOCode() << GlobalCom::getGlobalVariablesUBOCode();
    fragShaderString << counterBlockBufferShaderCodeString(GlobalCom::COUNTER_SSBO_BINDING) << logFunctionAndBufferShaderCodeString(GlobalCom::LOG_SSBO_BINDING);
    voxelizeGridShader.generateShader(fragShaderString, "./Shaders/VoxelizeGrid.frag", ShaderProgram::FRAGMENT);
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

void RenderToGrid::run(Scene& inputScene, GLBufferObject<CounterBlock> & ssboCounterSet, GLuint voxelizeMatrixBlock, GLuint logUniformBlock, GLBufferObject<LogStruct> & ssboLogList, GLuint texture3DColor, GLuint texture3DNormal)
{
    GLuint currentShaderProgram = voxelizeGridShader.use();

    glBindImageTexture(4, texture3DColor, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(5, texture3DNormal, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(7, texture3DCounterList, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

    glViewport(0, 0, 512, 512);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    inputScene.render(currentShaderProgram);
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

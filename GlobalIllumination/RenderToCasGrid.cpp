#include "RenderToCasGrid.h"

void RenderToCasGrid::initialize()
{
    if (hasInitialized) {
        return;
    }
    std::stringstream vertShaderString;
    vertShaderString << GenericShaderCodeString::vertHeader << GenericShaderCodeString::vertGeomOutput;
    vertShaderString << voxelizeBlockString(0) << GenericShaderCodeString::genericLimitsUniformBlock(7);
    vertShaderString << counterBlockBufferShaderCodeString(1) << logFunctionAndBufferShaderCodeString(7);

    shader.generateShader(vertShaderString, "./Shaders/Voxelize.vert", ShaderProgram::VERTEX);

    std::stringstream geomShaderString;
    geomShaderString << GenericShaderCodeString::geomHeader << GenericShaderCodeString::geomFragOutput;
    geomShaderString << voxelizeBlockString(0);
    shader.generateShader(geomShaderString, "./Shaders/Voxelize.geom", ShaderProgram::GEOMETRY);

    std::stringstream fragShaderString;
    fragShaderString << GenericShaderCodeString::fragHeader;
    fragShaderString << GenericShaderCodeString::genericLimitsUniformBlock(7);
    fragShaderString << counterBlockBufferShaderCodeString(1) << logFunctionAndBufferShaderCodeString(7);
    shader.generateShader(fragShaderString, "./Shaders/VoxelizeCasGrid.frag", ShaderProgram::FRAGMENT);
    shader.linkCompileValidate();
}

void RenderToCasGrid::run(Scene & inputScene, GLBufferObject<CounterBlock>& ssboCounterSet, glm::mat4 & worldToVoxelMat, GLuint logUniformBlock, GLBufferObject<LogStruct>& ssboLogList, CascadedGrid & casGrid)
{
    auto & cam = inputScene.cam;
    auto refPos = worldToVoxelMat * glm::vec4(cam.getPosition() - 2.0f * cam.getForward(), 1.0f);
    casGrid.setRefCamPosition(refPos, worldToVoxelMat, cam.getForward());

    GLuint currentShaderProgram = shader.use();
    glBindBufferBase(GL_UNIFORM_BUFFER, 7, logUniformBlock);

    ssboCounterSet.bind(1);
    ssboLogList.bind(7);

    GLuint dim = casGrid.getClipDimensions();
    GLuint numOfGrid = casGrid.getCascadedLevels();
    auto& voxelMatrixBlockIds = casGrid.getVoxelMatrixBlockIds();
    auto& textureColors = casGrid.getCasGridTextureIds(CascadedGrid::GridType::COLOR);
    auto& textureNormals = casGrid.getCasGridTextureIds(CascadedGrid::GridType::NORMAL);


    glViewport(0, 0, dim, dim);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);

    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    for (int i = 0; i < numOfGrid; i++) {
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, voxelMatrixBlockIds[i]); 
        glBindImageTexture(4, textureColors[i], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
        glBindImageTexture(5, textureNormals[i], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        inputScene.render(currentShaderProgram);
    }
}

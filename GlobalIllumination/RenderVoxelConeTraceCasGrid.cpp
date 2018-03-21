#include "RenderVoxelConeTraceCasGrid.h"



void RenderVoxelConeTraceCasGrid::initialize()
{
    if (hasInitialized) {
        return;
    }
    std::stringstream vertShaderString;
    vertShaderString << GenericShaderCodeString::vertHeader << GenericShaderCodeString::vertFragOutputNoTexCoord;
    shader.generateShader(vertShaderString, "./Shaders/VoxelConeTracingRender.vert", ShaderProgram::VERTEX);

    std::stringstream fragShaderString;
    fragShaderString << GenericShaderCodeString::fragHeaderNoTexCoord;
    fragShaderString << GenericShaderCodeString::genericLimitsUniformBlock(7);
    fragShaderString << counterBlockBufferShaderCodeString(1) << logFunctionAndBufferShaderCodeString(7);
    fragShaderString << voxelizeBlockString(3) << voxelizeCascadedBlockString(4);

    shader.generateShader(fragShaderString, "./Shaders/VoxelConeTracingCasGridRender.frag", ShaderProgram::FRAGMENT);
    shader.linkCompileValidate();
}

void RenderVoxelConeTraceCasGrid::run(Scene & inputScene, GLBufferObject<CounterBlock>& ssboCounterSet, CascadedGrid & cascadedGrid)
{
    shader.use();
    auto & res = inputScene.cam.getResolution();
    glViewport(0, 0, res.x, res.y);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    auto & colorCasGrid = cascadedGrid.getCasGridTextureIds(CascadedGrid::GridType::COLOR);
    auto & normalCasGrid = cascadedGrid.getCasGridTextureIds(CascadedGrid::GridType::NORMAL);
    auto & lightDirCasGrid = cascadedGrid.getCasGridTextureIds(CascadedGrid::GridType::LIGHT_DIRECTION);
    auto & lightEnergyCasGrid = cascadedGrid.getCasGridTextureIds(CascadedGrid::GridType::LIGHT_ENERGY);
    
    for (int i = 0; i < 3; i++) {
        glBindTextureUnit(4 * i + 0, colorCasGrid[i]);
        glBindTextureUnit(4 * i + 1, normalCasGrid[i]);
        glBindTextureUnit(4 * i + 2, lightDirCasGrid[i]);
        glBindTextureUnit(4 * i + 3, lightEnergyCasGrid[i]);
    }

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, inputScene.getMatrixBuffer()); //scene cam matrices
    ssboCounterSet.bind(1);

    inputScene.render(shader.getProgramId());
}

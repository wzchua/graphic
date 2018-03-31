#include "RenderLightIntoCasGrid.h"



void RenderLightIntoCasGrid::initialize()
{
    if (hasInitialized) {
        return;
    }
    std::stringstream compShaderString;
    compShaderString << GlobalShaderComponents::getHeader() << GlobalShaderComponents::getComputeShaderInputLayout(mWorkGroupSize.x, mWorkGroupSize.y);
    compShaderString << voxelizeBlockString(GlobalShaderComponents::VOXELIZATION_MATRIX_UBO_BINDING) << voxelizeCascadedBlockString(GlobalShaderComponents::CASGRID_VOXELIZATION_INFO_UBO_BINDING) << Scene::getLightUBOCode(GlobalShaderComponents::LIGHT_UBO_BINDING);
    compShaderString << counterBlockBufferShaderCodeString(GlobalShaderComponents::COUNTER_SSBO_BINDING) << logFunctionAndBufferShaderCodeString(GlobalShaderComponents::LOG_SSBO_BINDING);
    shader.generateShader(compShaderString, "./Shaders/LightInjectionIntoCasGrid.comp", ShaderProgram::COMPUTE);
    shader.linkCompileValidate();
}

void RenderLightIntoCasGrid::run(Scene & inputScene,  GLuint voxelizeMatrixBlock, CascadedGrid & cascadedGrid)
{
    auto& textureLightDirections = cascadedGrid.getCasGridTextureIds(CascadedGrid::GridType::LIGHT_DIRECTION);
    auto& textureLightEnergies = cascadedGrid.getCasGridTextureIds(CascadedGrid::GridType::LIGHT_ENERGY);

    shader.use();

    glBindBufferBase(GL_UNIFORM_BUFFER, GlobalShaderComponents::CASGRID_VOXELIZATION_INFO_UBO_BINDING, cascadedGrid.getVoxelizedCascadedBlockBufferId());

    glBindImageTexture(0, textureLightDirections[0], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(1, textureLightEnergies[0], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(2, textureLightDirections[1], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(3, textureLightEnergies[1], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(4, textureLightDirections[2], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glBindImageTexture(5, textureLightEnergies[2], 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
    glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT);

    int numOfPointLight = inputScene.getTotalPointLights();

    for (int i = 0; i < numOfPointLight; i++) {
        glBindBufferBase(GL_UNIFORM_BUFFER, GlobalShaderComponents::LIGHT_UBO_BINDING, inputScene.getPointLightBufferId(i));
        for (int j = 0; j < 6; j++) {
            RSM& rsm = inputScene.getPointLightRSM(i, j);
            auto res = rsm.getResolution();
            glBindTextureUnit(0, rsm.getVoxelPositionMap());
            glBindTextureUnit(1, rsm.getNormalMap());
            glDispatchCompute(res.x / mWorkGroupSize.x, res.y / mWorkGroupSize.y, 1);
        }
    }

    int numOfDirectionalLight = inputScene.getTotalDirectionalLights();
    for (int i = 0; i < numOfDirectionalLight; i++) {
        glBindBufferBase(GL_UNIFORM_BUFFER, GlobalShaderComponents::LIGHT_UBO_BINDING, inputScene.getDirectionalLightBufferId(i));
        RSM& rsm = inputScene.getDirectionalLightRSM(i);
        auto res = rsm.getResolution();
        glBindTextureUnit(0, rsm.getVoxelPositionMap());
        glBindTextureUnit(1, rsm.getNormalMap());
        glDispatchCompute(res.x / mWorkGroupSize.x, res.y / mWorkGroupSize.y, 1);
    }
}

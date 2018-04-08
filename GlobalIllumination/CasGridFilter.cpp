#include "CasGridFilter.h"
#include "CounterBlock.h"
#include "logStruct.h"

void CasGridFilter::initialize()
{
    if (hasInitialized) {
        return;
    }

    std::stringstream compShaderStringPreFilter;
    compShaderStringPreFilter << GlobalShaderComponents::getHeader() << GlobalShaderComponents::getComputeShaderInputLayout(mWorkGroupSize.x, mWorkGroupSize.y, mWorkGroupSize.z);
    compShaderStringPreFilter << counterBlockBufferShaderCodeString(GlobalShaderComponents::COUNTER_SSBO_BINDING) << logFunctionAndBufferShaderCodeString(GlobalShaderComponents::LOG_SSBO_BINDING);
    mPreFilterShader.generateShader(compShaderStringPreFilter, "./Shaders/MipmapCasGridPreFilter.comp", ShaderProgram::COMPUTE);
    mPreFilterShader.linkCompileValidate();

    std::stringstream compShaderString;
    compShaderString << GlobalShaderComponents::getHeader() << GlobalShaderComponents::getComputeShaderInputLayout(mWorkGroupSize.x, mWorkGroupSize.y, mWorkGroupSize.z);
    compShaderString << counterBlockBufferShaderCodeString(GlobalShaderComponents::COUNTER_SSBO_BINDING) << logFunctionAndBufferShaderCodeString(GlobalShaderComponents::LOG_SSBO_BINDING);
    shader.generateShader(compShaderString, "./Shaders/MipmapCasGrid.comp", ShaderProgram::COMPUTE);
    shader.linkCompileValidate();
}

void CasGridFilter::run(CascadedGrid & casGrid)
{
    mPreFilterShader.use();
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    GLuint dim = casGrid.getClipDimensions();
    glm::ivec3 setOfWorkGroups = glm::ivec3(ceil(dim * 1.0 / mWorkGroupSize.x), ceil(dim * 1.0 / mWorkGroupSize.y), ceil(dim * 1.0 / mWorkGroupSize.z));

    auto & colorCasGrid = casGrid.getCasGridTextureIds(CascadedGrid::GridType::COLOR);
    auto & normalCasGrid = casGrid.getCasGridTextureIds(CascadedGrid::GridType::NORMAL);
    auto & lightDirCasGrid = casGrid.getCasGridTextureIds(CascadedGrid::GridType::LIGHT_DIRECTION);
    auto & lightEnergyCasGrid = casGrid.getCasGridTextureIds(CascadedGrid::GridType::LIGHT_ENERGY);

    for (int i = 0; i < 3; i++) {
        glBindImageTexture(0, colorCasGrid[i], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);
        glBindImageTexture(1, normalCasGrid[i], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8_SNORM);
        glBindImageTexture(2, lightDirCasGrid[i], 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8_SNORM);
        glDispatchCompute(setOfWorkGroups.x, setOfWorkGroups.y, setOfWorkGroups.z);
    }

    setOfWorkGroups = glm::ivec3(ceil(dim * 0.5 / mWorkGroupSize.x), ceil(dim * 0.5 / mWorkGroupSize.y), ceil(dim * 0.5 / mWorkGroupSize.z));
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
    shader.use();    
    //level 0
    GLuint level = 0;
    glBindImageTexture(0, colorCasGrid[level], 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8);
    glBindImageTexture(1, normalCasGrid[level], 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8_SNORM);
    glBindImageTexture(2, lightDirCasGrid[level], 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8_SNORM);
    glBindImageTexture(3, lightEnergyCasGrid[level], 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);

    glBindImageTexture(4, colorCasGrid[level], 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glBindImageTexture(5, normalCasGrid[level], 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8_SNORM);
    glBindImageTexture(6, lightDirCasGrid[level], 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8_SNORM);
    glBindImageTexture(7, lightEnergyCasGrid[level], 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);
    glDispatchCompute(setOfWorkGroups.x, setOfWorkGroups.y, setOfWorkGroups.z);

    level = 1;
    glBindImageTexture(0, colorCasGrid[level], 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8);
    glBindImageTexture(1, normalCasGrid[level], 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8_SNORM);
    glBindImageTexture(2, lightDirCasGrid[level], 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8_SNORM);
    glBindImageTexture(3, lightEnergyCasGrid[level], 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);

    glBindImageTexture(4, colorCasGrid[level], 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glBindImageTexture(5, normalCasGrid[level], 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8_SNORM);
    glBindImageTexture(6, lightDirCasGrid[level], 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8_SNORM);
    glBindImageTexture(7, lightEnergyCasGrid[level], 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);
    glDispatchCompute(setOfWorkGroups.x, setOfWorkGroups.y, setOfWorkGroups.z);

    GLuint mipLevel = 10 - casGrid.getCascadedLevels();
    level = 2;
    for (int i = 1; i < mipLevel; i++) {
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        int lowerLevel = i - 1;
        glBindImageTexture(0, colorCasGrid[level], lowerLevel, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8);
        glBindImageTexture(1, normalCasGrid[level], lowerLevel, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8_SNORM);
        glBindImageTexture(2, lightDirCasGrid[level], lowerLevel, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8_SNORM);
        glBindImageTexture(3, lightEnergyCasGrid[level], lowerLevel, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);

        glBindImageTexture(4, colorCasGrid[level], i, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
        glBindImageTexture(5, normalCasGrid[level], i, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8_SNORM);
        glBindImageTexture(6, lightDirCasGrid[level], i, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8_SNORM);
        glBindImageTexture(7, lightEnergyCasGrid[level], i, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);
        glDispatchCompute(setOfWorkGroups.x, setOfWorkGroups.y, setOfWorkGroups.z);
    }
}

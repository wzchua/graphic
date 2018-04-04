#include "CasGridFilter.h"

void CasGridFilter::initialize()
{
    if (hasInitialized) {
        return;
    }

    std::stringstream compShaderString;
    compShaderString << GlobalShaderComponents::getHeader() << GlobalShaderComponents::getComputeShaderInputLayout(mWorkGroupSize.x, mWorkGroupSize.y, mWorkGroupSize.z);
    shader.generateShader(compShaderString, "./Shaders/MipmapCasGrid.comp", ShaderProgram::COMPUTE);
    shader.linkCompileValidate();
}

void CasGridFilter::run(CascadedGrid & casGrid)
{
    shader.use();
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    GLuint dim = casGrid.getClipDimensions();
    glm::ivec3 setOfWorkGroups = glm::ivec3(ceil(dim * 0.5 / mWorkGroupSize.x), ceil(dim * 0.5 / mWorkGroupSize.y), ceil(dim * 0.5 / mWorkGroupSize.z));

    auto & colorCasGrid = casGrid.getCasGridTextureIds(CascadedGrid::GridType::COLOR);
    auto & normalCasGrid = casGrid.getCasGridTextureIds(CascadedGrid::GridType::NORMAL);
    auto & lightDirCasGrid = casGrid.getCasGridTextureIds(CascadedGrid::GridType::LIGHT_DIRECTION);
    auto & lightEnergyCasGrid = casGrid.getCasGridTextureIds(CascadedGrid::GridType::LIGHT_ENERGY);
    
    //level 0
    GLuint level = 0;
    glBindImageTexture(0, colorCasGrid[level], 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8);
    glBindImageTexture(1, normalCasGrid[level], 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8);
    glBindImageTexture(2, lightDirCasGrid[level], 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8);
    glBindImageTexture(3, lightEnergyCasGrid[level], 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);

    glBindImageTexture(4, colorCasGrid[level], 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glBindImageTexture(5, normalCasGrid[level], 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glBindImageTexture(6, lightDirCasGrid[level], 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glBindImageTexture(7, lightEnergyCasGrid[level], 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);
    glDispatchCompute(setOfWorkGroups.x, setOfWorkGroups.y, setOfWorkGroups.z);

    level = 1;
    glBindImageTexture(0, colorCasGrid[level], 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8);
    glBindImageTexture(1, normalCasGrid[level], 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8);
    glBindImageTexture(2, lightDirCasGrid[level], 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8);
    glBindImageTexture(3, lightEnergyCasGrid[level], 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);

    glBindImageTexture(4, colorCasGrid[level], 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glBindImageTexture(5, normalCasGrid[level], 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glBindImageTexture(6, lightDirCasGrid[level], 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
    glBindImageTexture(7, lightEnergyCasGrid[level], 1, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);
    glDispatchCompute(setOfWorkGroups.x, setOfWorkGroups.y, setOfWorkGroups.z);

    GLuint mipLevel = 10 - casGrid.getCascadedLevels();
    level = 2;
    for (int i = 1; i < mipLevel; i++) {
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        int lowerLevel = i - 1;
        glBindImageTexture(0, colorCasGrid[level], lowerLevel, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8);
        glBindImageTexture(1, normalCasGrid[level], lowerLevel, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8);
        glBindImageTexture(2, lightDirCasGrid[level], lowerLevel, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA8);
        glBindImageTexture(3, lightEnergyCasGrid[level], lowerLevel, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);

        glBindImageTexture(4, colorCasGrid[level], i, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
        glBindImageTexture(5, normalCasGrid[level], i, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
        glBindImageTexture(6, lightDirCasGrid[level], i, GL_TRUE, 0, GL_WRITE_ONLY, GL_RGBA8);
        glBindImageTexture(7, lightEnergyCasGrid[level], i, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);
        glDispatchCompute(setOfWorkGroups.x, setOfWorkGroups.y, setOfWorkGroups.z);
    }
}

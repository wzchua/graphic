#include "FilterCasGrid.h"



void FilterCasGrid::run(CascadedGrid & cascadedGrid)
{
    GLuint highestLevel = cascadedGrid.getCascadedLevels() - 1;
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glGenerateTextureMipmap(cascadedGrid.getCasGridTextureIds(CascadedGrid::GridType::COLOR)[highestLevel]);
    glGenerateTextureMipmap(cascadedGrid.getCasGridTextureIds(CascadedGrid::GridType::NORMAL)[highestLevel]);
    glGenerateTextureMipmap(cascadedGrid.getCasGridTextureIds(CascadedGrid::GridType::LIGHT_DIRECTION)[highestLevel]);
    glGenerateTextureMipmap(cascadedGrid.getCasGridTextureIds(CascadedGrid::GridType::LIGHT_ENERGY)[highestLevel]);
}

FilterCasGrid::FilterCasGrid()
{
}


FilterCasGrid::~FilterCasGrid()
{
}

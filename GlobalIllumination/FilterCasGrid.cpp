#include "FilterCasGrid.h"



void FilterCasGrid::run(GLuint numOfGrid, GLuint textureColors[3], GLuint textureNormal[3], GLuint textureLightDir[3], GLuint textureLightEnergy[3])
{
    GLuint highestLevel = numOfGrid - 1;
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    glGenerateTextureMipmap(textureColors[highestLevel]);
    glGenerateTextureMipmap(textureNormal[highestLevel]);
    glGenerateTextureMipmap(textureLightDir[highestLevel]);
    glGenerateTextureMipmap(textureLightEnergy[highestLevel]);
}

FilterCasGrid::FilterCasGrid()
{
}


FilterCasGrid::~FilterCasGrid()
{
}

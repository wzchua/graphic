#pragma once
#include "ShaderProgram.h"
class FilterCasGrid
{
private:
    bool hasInitialized = false;
    ShaderProgram shader;

public:
    void initialize();
    void run(GLuint numOfGrid, GLuint textureColors[3], GLuint textureNormal[3], GLuint textureLightDir[3], GLuint textureLightEnergy[3]);
    FilterCasGrid();
    ~FilterCasGrid();
};


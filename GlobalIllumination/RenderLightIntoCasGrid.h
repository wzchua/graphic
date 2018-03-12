#pragma once
#include "ShaderProgram.h"
#include "Scene.h"
class RenderLightIntoCasGrid
{
private:
    bool hasInitialized = false;
    ShaderProgram shader;
public:
    void initialize();
    void run(Scene & inputScene, GLuint numOfGrid, GLuint textureLightDirections[3], GLuint textureLightEnergies[3], GLuint voxelizeCasGridBlocks[3]);
    RenderLightIntoCasGrid();
    ~RenderLightIntoCasGrid();
};


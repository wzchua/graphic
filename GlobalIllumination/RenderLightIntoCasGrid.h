#pragma once
#include "ShaderProgram.h"
#include "Scene.h"
#include "CascadedGrid.h"
class RenderLightIntoCasGrid
{
private:
    bool hasInitialized = false;
    ShaderProgram shader;
public:
    void initialize();
    void run(Scene & inputScene, GLuint voxelizeMatrixBlock, CascadedGrid & cascadedGrid);
    RenderLightIntoCasGrid();
    ~RenderLightIntoCasGrid();
};


#pragma once
#include "AbstractModule.h"
#include "Scene.h"
#include "CascadedGrid.h"
class RenderLightIntoCasGrid : public AbstractModule
{
public:
    void initialize();
    void run(Scene & inputScene, GLuint voxelizeMatrixBlock, CascadedGrid & cascadedGrid);
};


#pragma once
#include "AbstractModule.h"
#include "Scene.h"
#include "CascadedGrid.h"
class RenderLightIntoCasGrid : public AbstractModule
{
private:
    GLuint rboId;
    GLuint fboId;
    const glm::ivec2 rsmRes{ 1024, 1024 };
public:
    void initialize();
    void run(Scene & inputScene, GLuint voxelizeMatrixBlock, CascadedGrid & cascadedGrid);
};


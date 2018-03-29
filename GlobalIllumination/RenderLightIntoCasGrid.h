#pragma once
#include "AbstractModule.h"
#include "Scene.h"
#include "CascadedGrid.h"
#include "CounterBlock.h"
#include "LogStruct.h"
#include "GLBufferObject.h"
#include "VoxelizeCascadedBlock.h"
#include "VoxelizeBlock.h"
class RenderLightIntoCasGrid : public AbstractModule
{
private:
    const glm::ivec3 mWorkGroupSize{ 32, 32, 1 };
public:
    void initialize();
    void run(Scene & inputScene, GLuint voxelizeMatrixBlock, CascadedGrid & cascadedGrid);
};


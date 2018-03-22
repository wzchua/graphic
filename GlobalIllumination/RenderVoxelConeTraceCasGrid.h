#pragma once
#include "AbstractModule.h"
#include "GenericShaderCodeString.h"
#include "Scene.h"
#include "GLBufferObject.h"
#include "CounterBlock.h"
#include "CascadedGrid.h"
#include "LogStruct.h"
#include "OpenGLQueryObject.h"
#include "CameraBlock.h"

class RenderVoxelConeTraceCasGrid : public AbstractModule
{
private:
    CameraBlock camBlk;
    GLuint camBlkBufferId;
public:
    void initialize();
    void run(Scene & inputScene, GLuint voxelizeMatrixBlock, GLBufferObject<CounterBlock> & ssboCounterSet, CascadedGrid & cascadedGrid);
};


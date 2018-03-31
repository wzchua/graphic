#pragma once
#include "AbstractModule.h"
#include "Scene.h"
#include "GLBufferObject.h"
#include "CounterBlock.h"
#include "CascadedGrid.h"
#include "LogStruct.h"
#include "GlobalShaderComponents.h"
#include "CameraBlock.h"
#include "GBuffer.h"

typedef GlobalShaderComponents GlobalCom;
class RenderVoxelConeTraceCasGrid : public AbstractModule
{
private:
    const glm::ivec3 mWorkGroupSize{ 32, 32, 1 };
    CameraBlock camBlk;
    GLuint camBlkBufferId;
public:
    void initialize();
    void run(Scene & inputScene, GLBufferObject<CounterBlock> & ssboCounterSet, CascadedGrid & cascadedGrid, GLBufferObject<LogStruct> & logList, GBuffer & gBuffer);
};


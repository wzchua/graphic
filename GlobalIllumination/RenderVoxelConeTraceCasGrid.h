#pragma once
#include "AbstractModule.h"
#include "GenericShaderCodeString.h"
#include "Scene.h"
#include "GLBufferObject.h"
#include "CounterBlock.h"
#include "CascadedGrid.h"
#include "LogStruct.h"

class RenderVoxelConeTraceCasGrid : public AbstractModule
{
public:
    void initialize();
    void run(Scene & inputScene, GLBufferObject<CounterBlock> & ssboCounterSet, CascadedGrid & cascadedGrid);
};


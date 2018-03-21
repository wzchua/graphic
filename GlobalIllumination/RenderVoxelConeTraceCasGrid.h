#pragma once
#include "AbstractModule.h"
#include "Scene.h"
#include "GLBufferObject.h"
#include "CounterBlock.h"
#include "CascadedGrid.h"

class RenderVoxelConeTraceCasGrid : public AbstractModule
{
private:
    GLuint quadVAOId;
    GLuint quadVBOId;
    GLuint quadEBOId;
public:
    void initialize();
    void run(Scene & inputScene, GLBufferObject<CounterBlock> & ssboCounterSet, CascadedGrid & cascadedGrid);
};


#pragma once
#include "ShaderProgram.h"
#include "Scene.h"
#include "GLBufferObject.h"
#include "CounterBlock.h"
#include "CascadedGrid.h"

class RenderVoxelConeTraceCasGrid
{
private:
    bool hasInitialized = false;
    ShaderProgram shader;

    GLuint quadVAOId;
    GLuint quadVBOId;
    GLuint quadEBOId;
public:
    void initialize();
    void run(Scene & inputScene, GLBufferObject<CounterBlock> & ssboCounterSet, CascadedGrid & cascadedGrid);
    RenderVoxelConeTraceCasGrid();
    ~RenderVoxelConeTraceCasGrid();
};


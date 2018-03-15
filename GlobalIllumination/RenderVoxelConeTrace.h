#pragma once
#include <glm/glm.hpp>
#include "ShaderProgram.h"
#include "Scene.h"
#include "GLBufferObject.h"
#include "CounterBlock.h"
#include "Octree.h"

class RenderVoxelConeTrace
{
private:
    ShaderProgram shader;
    bool hasInitialized = false;

    GLuint quadVAOId;
    GLuint quadVBOId;
    GLuint quadEBOId;
public:
    void initialize();
    void run(Scene & inputScene, GLBufferObject<CounterBlock> & counterBlk, Octree & octree);
    RenderVoxelConeTrace();
    ~RenderVoxelConeTrace();
};


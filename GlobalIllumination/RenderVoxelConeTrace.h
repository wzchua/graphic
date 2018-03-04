#pragma once
#include <glm/glm.hpp>
#include "ShaderProgram.h"
#include "Scene.h"
#include "GLBufferObject.h"
#include "NodeStruct.h"
#include "CounterBlock.h"

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
    void run(Scene & inputScene, GLBufferObject<CounterBlock> & counterBlk, GLBufferObject<NodeStruct> & octree, GLuint textureBrickColor, GLuint textureBrickNormal, GLuint textureBrickLightDir, GLuint textureBrickLightEnergy);
    RenderVoxelConeTrace();
    ~RenderVoxelConeTrace();
};


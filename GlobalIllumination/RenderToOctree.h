#pragma once
#include "VoxelizeBlock.h"
#include "ShaderProgram.h"
#include "Scene.h"
#include "GLBufferObject.h"
#include "CounterBlock.h"
#include "FragStruct.h"
#include "LogStruct.h"
#include "Octree.h"
#include "AbstractModule.h"

class RenderToOctree : public AbstractModule
{
public:
    void initialize();
    void run(Scene& inputScene, Octree & octree);
};


#pragma once
#include "GenericShaderCodeString.h"
#include "VoxelizeBlock.h"
#include "ShaderProgram.h"
#include "Scene.h"
#include "GLBufferObject.h"
#include "CounterBlock.h"
#include "FragStruct.h"
#include "LogStruct.h"
#include "Octree.h"

class RenderToOctree
{
private:

    ShaderProgram voxelizeOctreeShader;
    bool hasInitialized = false;
public:
    void initialize();
    void run(Scene& inputScene, GLBufferObject<CounterBlock> & counterSet, GLuint voxelizeMatrixBlock, GLuint logUniformBlock, Octree & octree,
                GLBufferObject<FragStruct> & ssboFragList, GLBufferObject<LogStruct> & ssboLogList);
    RenderToOctree();
    ~RenderToOctree();
};


#pragma once
#include "Scene.h"
#include "ShaderProgram.h"
#include "Octree.h"

class RenderLightIntoOctree
{
private:
    ShaderProgram injectLightOctreeShader;
    bool hasInitialized = false;
public:
    void initialize();
    void run(Scene& inputScene, Octree & octree, GLuint voxelizeMatrixBlock);
    RenderLightIntoOctree();
    ~RenderLightIntoOctree();
};


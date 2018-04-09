#pragma once
#include "Scene.h"
#include "ShaderProgram.h"
#include "Octree.h"
#include "AbstractModule.h"

class RenderLightIntoOctree : public AbstractModule
{
private:
    const glm::ivec3 mWorkGroupSize{ 32, 32, 1 };
public:
    void initialize();
    void run(Scene& inputScene, Octree & octree, GLuint voxelizeMatrixBlock);
};


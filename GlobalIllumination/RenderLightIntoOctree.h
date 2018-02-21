#pragma once
#include "Scene.h"
#include "NodeStruct.h"
#include "GLBufferObject.h"
class RenderLightIntoOctree
{
private:

    bool hasInitialized = false;
public:
    void initialize();
    void run(Scene& inputScene, GLBufferObject<NodeStruct> & nodeOctree, GLuint textureLight, GLuint voxelizeMatrixBlock);
    RenderLightIntoOctree();
    ~RenderLightIntoOctree();
};


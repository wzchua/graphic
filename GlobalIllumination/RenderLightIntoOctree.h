#pragma once
#include "Scene.h"
#include "NodeStruct.h"
#include "GLBufferObject.h"
#include "ShaderProgram.h"
class RenderLightIntoOctree
{
private:
    ShaderProgram injectLightOctreeShader;
    bool hasInitialized = false;
public:
    void initialize();
    void run(Scene& inputScene, GLBufferObject<NodeStruct> & nodeOctree, GLuint textureLight, GLuint textureLightDir, GLuint voxelizeMatrixBlock);
    RenderLightIntoOctree();
    ~RenderLightIntoOctree();
};


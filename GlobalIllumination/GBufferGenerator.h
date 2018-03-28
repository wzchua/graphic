#pragma once
#include "AbstractModule.h"
#include "Scene.h"
#include "GBuffer.h"
class GBufferGenerator : public AbstractModule
{
public:
    void initialize();
    void run(Scene & inputScene, GBuffer & gBuffer, GLuint voxelMatrixUniformBuffer);
    GBufferGenerator();
    ~GBufferGenerator();
};


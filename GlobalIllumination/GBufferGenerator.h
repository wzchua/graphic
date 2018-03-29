#pragma once
#include "GlobalShaderComponents.h"
#include "AbstractModule.h"
#include "Scene.h"
#include "GBuffer.h"
#include "VoxelizeBlock.h"
typedef GlobalShaderComponents GlobalCom;
class GBufferGenerator : public AbstractModule
{
public:
    void initialize();
    void run(Scene & inputScene, GBuffer & gBuffer);
    GBufferGenerator();
    ~GBufferGenerator();
};


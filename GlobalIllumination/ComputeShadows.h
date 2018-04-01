#pragma once
#include <glm\glm.hpp>
#include "AbstractModule.h"
#include "GBuffer.h"
#include "Scene.h"
#include "CameraBlock.h"
class ComputeShadows : public AbstractModule
{
private:
    const glm::ivec3 mWorkGroupSize{ 32, 32, 1 };
    CameraBlock camBlk;
    GLuint camBlkBufferId;
public:
    void initialize();
    void run(Scene & inputScene, GBuffer & gBuffer);
};


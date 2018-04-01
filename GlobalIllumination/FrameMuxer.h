#pragma once
#include <glm\glm.hpp>
#include "AbstractModule.h"
#include "GlobalShaderComponents.h"
#include "GBuffer.h"
class FrameMuxer : public AbstractModule
{
private:
    GLuint fboId;
    const glm::ivec3 mWorkGroupSize{ 32, 32, 1 };
public:
    void initialize();
    void run(GBuffer& gBuffer);
};


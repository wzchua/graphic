#pragma once
#include "glad\glad.h"
#include "ShaderProgram.h"
#include "Scene.h"

class RenderWithShadows
{
private:
    ShaderProgram mRSMShader;
    ShaderProgram mRenderWithShadows;
    glm::ivec2 mRSMResolution{ 1024, 1024 };
public:
    //void generateShadowMaps(Scene & inputScene);
    RenderWithShadows();
    ~RenderWithShadows();
};

